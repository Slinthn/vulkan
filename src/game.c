/**
 * @brief Main game code 
 *  
 */

#define SLN_WINDOW_WIDTH 1280
#define SLN_WINDOW_HEIGHT 720

#include "macros.c"

#define VK_FRAMEBUFFER_COUNT 2

#include "graphics/vulkan.c"
#include "file.c"

struct sln_resources {
  struct vk_shader shader;
};

static struct vk_state vulkan;
static struct sln_resources resources;

void sln_init(struct vk_surface surface) {

  struct vk_initialise_info init_info = {0};
  init_info.extent.width = SLN_WINDOW_WIDTH;
  init_info.extent.height = SLN_WINDOW_HEIGHT;
  init_info.format.format = VK_FORMAT_R8G8B8A8_SRGB;
  init_info.format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  init_info.vulkan_version = VK_API_VERSION_1_0;
  init_info.surface = surface;
  init_info.framebuffer_count = VK_FRAMEBUFFER_COUNT;

  vulkan = vk_init(init_info);

  struct sln_file vertex_file = sln_read_file("shader-v.spv", 4);
  struct sln_file fragment_file = sln_read_file("shader-f.spv", 4);

  struct vk_shader_info shader_info = {0};
  shader_info.device = vulkan.device;
  shader_info.render_pass = vulkan.render_pass;
  shader_info.vertex_data = vertex_file.data;
  shader_info.vertex_data_size = vertex_file.allocated_size;
  shader_info.fragment_data = fragment_file.data;
  shader_info.fragment_data_size = fragment_file.allocated_size;

  resources.shader = vk_create_shader(shader_info);
    
  sln_close_file(vertex_file);
  sln_close_file(fragment_file);
}

void sln_update(struct sln_app app) {

  vk_render_begin(&vulkan, (float[4]){1, 1, 1, 1});

  vk_render_bind_shader(vulkan, resources.shader);

  vk_render_set_viewport(vulkan, app.width, app.height);

  vkCmdDraw(vulkan.command_buffer, 3, 1, 0, 0);

  vk_render_end(vulkan);
}
