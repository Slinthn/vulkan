/**
 * @brief Main game code 
 *  
 */

#define SLN_WINDOW_WIDTH 1280
#define SLN_WINDOW_HEIGHT 720
#define SLN_FRAMEBUFFER_COUNT 2

#include "macros.c"
#include "graphics/vulkan_init.c"
#include "file.c"

struct sln_resources {
  struct vk_shader shader;
  struct vk_buffer vertex_buffer;
};

static struct vk_state vulkan;
static struct sln_resources resources;

void sln_load_shaders(void) {

  struct sln_file vertex_file = sln_read_file("shader-v.spv", 4);
  struct sln_file fragment_file = sln_read_file("shader-f.spv", 4);

  resources.shader = vk_create_shader(&vulkan, vertex_file.data,
    vertex_file.allocated_size, fragment_file.data,
    fragment_file.allocated_size);
    
  sln_close_file(vertex_file);
  sln_close_file(fragment_file);
}

void sln_load_models(void) {

  struct vk_vertex vertices[] = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
  };

  resources.vertex_buffer = vk_create_vertex_buffer(&vulkan, vertices,
    sizeof(vertices));
}

void sln_init(struct vk_surface surface) {

  vulkan = vk_init(surface);
  sln_load_shaders();
  sln_load_models();
}

void sln_update(struct sln_app app) {

  vk_render_begin(&vulkan, (float[4]){1, 1, 1, 1});
  vk_render_bind_shader(vulkan, resources.shader);
  vk_render_set_viewport(vulkan, app.width, app.height);

  VkDeviceSize offsets[1] = {0};

  vkCmdBindVertexBuffers(vulkan.command_buffer, 0, 1,
    &resources.vertex_buffer.buffer, offsets);

  vkCmdDraw(vulkan.command_buffer, 3, 1, 0, 0);
  vk_render_end(vulkan);
}
