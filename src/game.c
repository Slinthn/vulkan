/**
 * @brief Main game code 
 *  
 */

#include "macros.c"

struct sln_file {
  void *data;
  uint64_t size;
  uint64_t allocated_size;
};

union vk_queue_family {
  struct {
    uint32_t graphics;
    uint32_t present;
  } type;
  uint32_t families[2];
};

struct sln_vulkan_shader {
  VkPipeline pipeline;
};

struct sln_vulkan_state {
  VkInstance instance;

#ifdef SLN_DEBUG
  VkDebugUtilsMessengerEXT debug_messenger;
#endif

  VkPhysicalDevice physical_device;
  VkDevice device;
  VkSurfaceKHR surface;
  VkRenderPass render_pass;
  VkImageView views[2];
  VkFramebuffer framebuffers[2];
  union vk_queue_family queue_family;
  VkCommandPool command_pool;
  VkCommandBuffer command_buffer;
  VkQueue graphics_queue, present_queue;
  VkSwapchainKHR swapchain;
  VkExtent2D extent;
  VkSemaphore image_ready_semaphore, render_ready_semaphore;
  VkFence render_ready_fence;
  struct sln_vulkan_shader shader;
  uint32_t current_image_index;
  uint32_t unused0;
};

#include "graphics/vulkan.c"
#include "graphics/vulkan_render.c"
#include "file.c"

static struct sln_vulkan_state vulkan;

/**
 * @brief Initialises Vulkan. Should be called after program starts
 * 
 * @param hinstance Windows HINSTNACE
 * @param hwnd Windows HWND
 * @param extent Preferred dimensions of framebuffer
 * @param format Swapchain colour format
 * @return struct sln_vulkan_state A structure containing Vulkan details
 */
struct sln_vulkan_state sln_vulkan_init(struct sln_app app, VkExtent2D extent,
  VkSurfaceFormatKHR format) {

  struct sln_vulkan_state state = {0};
  state.extent = extent;

  vk_create_instance(VK_API_VERSION_1_0, &state.instance);

#ifdef SLN_DEBUG
  vk_create_debug_messenger(state.instance, &state.debug_messenger);
#endif

  vk_select_suitable_physical_device(state.instance, &state.physical_device);

#ifdef SLN_WIN64
  vk_win64(state.instance, app.hinstance, app.hwnd, &state.surface);
#else
  #error "No Vulkan surface has been selected."
#endif

  vk_select_appropriate_queue_families(state.physical_device, state.surface,
    &state.queue_family.type.graphics, &state.queue_family.type.present);

  vk_create_device_and_queue(state.physical_device,
    state.queue_family.families, SIZEOF_ARRAY(state.queue_family.families),
    &state.device);

  vkGetDeviceQueue(state.device, state.queue_family.type.graphics, 0,
    &state.graphics_queue);

  vkGetDeviceQueue(state.device, state.queue_family.type.present, 0,
    &state.present_queue);

  vk_create_swapchain(state.device, state.physical_device, state.surface,
  state.queue_family.families, &state.extent, &format, &state.swapchain);

  vk_create_render_pass(state.device, format.format, &state.render_pass);

  vk_create_command_pool(state.device, state.queue_family.type.graphics,
    &state.command_pool);

  vk_create_command_buffer(state.device, state.command_pool,
    &state.command_buffer);

  VkImage *images;
  uint32_t image_count;
  vk_get_swapchain_images(state.device, state.swapchain, &images,
    &image_count);

  for (uint32_t i = 0; i < image_count; i++) {
    vk_get_image_view(state.device, images[i], format.format, &state.views[i]);

    vk_create_framebuffer(state.device, state.extent, state.render_pass,
      state.views[i], &state.framebuffers[i]);
  }

  free(images);

  vk_create_semaphore(state.device, &state.image_ready_semaphore);
  vk_create_semaphore(state.device, &state.render_ready_semaphore);

  vk_create_fence(state.device, &state.render_ready_fence);

  return state;
}

struct sln_vulkan_shader sln_vulkan_create_shader(struct sln_vulkan_state state,
  char *vertex, char *fragment) {

  struct sln_vulkan_shader shader = {0};

  struct sln_file vertex_file = sln_read_file(vertex, 4);
  struct sln_file fragment_file = sln_read_file(fragment, 4);

  VkPipelineShaderStageCreateInfo vertex_stage = {0};
  vertex_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertex_stage.pName = "main";

  vk_create_shader_module(state.device, vertex_file.data,
    vertex_file.allocated_size, &vertex_stage.module);

  VkPipelineShaderStageCreateInfo fragment_stage = {0};
  fragment_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragment_stage.pName = "main";

  vk_create_shader_module(state.device, fragment_file.data,
    fragment_file.allocated_size, &fragment_stage.module);
  
  vk_create_graphics_pipeline(state.device, vertex_stage, fragment_stage,
    state.render_pass, &shader.pipeline);

  sln_close_file(vertex_file);
  sln_close_file(fragment_file);

  return shader;
}

void sln_init(struct sln_app app) {

  VkExtent2D extent = {0};
  extent.width = 1920;
  extent.height = 1080;

  VkSurfaceFormatKHR format = {0};
  format.format = VK_FORMAT_R8G8B8A8_SRGB;
  format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

  vulkan = sln_vulkan_init(app, extent, format);

  vulkan.shader = sln_vulkan_create_shader(vulkan, "shader-v.spv",
    "shader-f.spv");
}

void sln_update(struct sln_app app) {

  vkr_begin(vulkan, (float[4]){1, 1, 1, 1}, &vulkan.current_image_index);

  vkr_set_shader(vulkan, vulkan.shader);

  vkr_set_viewport(vulkan, app.width, app.height);

  vkCmdDraw(vulkan.command_buffer, 3, 1, 0, 0);

  vkr_end(vulkan);
}
