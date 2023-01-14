/**
 * @brief Main game code 
 *  
 */

#include "macros.c"
#include "graphics/vulkan.c"

struct sln_file {
  void *data;
  uint64_t size;
  uint64_t allocated_size;
};

/**
 * @brief Read the contents of a file to a buffer
 * 
 * @param filename The name of the file to read
 * @param alignment Whether the allocated memory size needs to be aligned
 * @return struct sln_file The created and read file
 */
struct sln_file sln_read_file(char *filename, uint64_t alignment) {

  struct sln_file ret = {0}; 
    
  FILE *file = fopen(filename, "rb");

  fseek(file, 0, SEEK_END);
  ret.size = ftell(file);
  fseek(file, 0, SEEK_SET);

  ret.allocated_size = ALIGN_UP(ret.size, alignment);

  ret.data = calloc(1, ret.allocated_size);
  fread(ret.data, ret.size, 1, file);

  fclose(file);

  return ret;
}

/**
 * @brief Delete the memory of a file
 * 
 * @param file The file of which the memory is to be freed
 */
void sln_close_file(struct sln_file file) {

  free(file.data);
}

// TODO: No bueno!!
static struct vk_state vulkan;

void sln_init(struct sln_app app) {

  VkSurfaceFormatKHR format = {0};
  format.format = VK_FORMAT_R8G8B8A8_SRGB;
  format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

  VkExtent2D extent = {0};
  extent.width = 1920;
  extent.height = 1080;

  vulkan = vk_init(app, extent, format);

  struct sln_file vertex_file = sln_read_file("shader-v.spv", 4);
  struct sln_file fragment_file = sln_read_file("shader-f.spv", 4);


  VkPipelineShaderStageCreateInfo vertex_stage = {0};
  vertex_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertex_stage.pName = "main";
  vk_create_shader_module(vulkan.device, vertex_file.data,
    vertex_file.allocated_size, &vertex_stage.module);

  VkPipelineShaderStageCreateInfo fragment_stage = {0};
  fragment_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragment_stage.pName = "main";
  vk_create_shader_module(vulkan.device, fragment_file.data,
    fragment_file.allocated_size, &fragment_stage.module);
  
  vk_create_graphics_pipeline(vulkan.device, vertex_stage, fragment_stage,
    vulkan.render_pass, &vulkan.pipeline);

  sln_close_file(vertex_file);
  sln_close_file(fragment_file);
}


void sln_update(void) {

  vkWaitForFences(vulkan.device, 1, &vulkan.render_ready_fence, 1, UINT64_MAX);
  vkResetFences(vulkan.device, 1, &vulkan.render_ready_fence);

  uint32_t image_index;
  vkAcquireNextImageKHR(vulkan.device, vulkan.swapchain, UINT64_MAX,
    vulkan.image_ready_semaphore, VK_NULL_HANDLE, &image_index);

  vk_begin_frame(vulkan.command_buffer, vulkan.extent,
    vulkan.framebuffers[image_index], vulkan.render_pass, vulkan.pipeline);

  vkCmdDraw(vulkan.command_buffer, 3, 1, 0, 0);

  vk_end_frame(vulkan.command_buffer, vulkan.swapchain,
    vulkan.image_ready_semaphore, vulkan.render_ready_semaphore,
    vulkan.render_ready_fence, vulkan.graphics_queue, vulkan.present_queue,
    image_index);
}