#define SLN_WINDOW_WIDTH 1920
#define SLN_WINDOW_HEIGHT 1080
#define SLN_FRAMEBUFFER_COUNT 2

#ifdef SLN_VULKAN
#pragma warning(push, 0)
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#pragma warning(pop)
#endif  // SLN_VULKAN

#include "macros.c"
#include "graphics/vulkan.c"
#include "file.c"

struct sln_model {
  struct vk_buffer vertex_buffer;
  struct vk_index_buffer index_buffer;
};

struct sln_resources {
  struct vk_shader shader;
  struct sln_model model;
  struct vk_buffer vertex_buffer;
  struct vk_index_buffer index_buffer;
};

#pragma pack(push, 1)
struct sln_vertex {
  float position[3];
  float texture[2];
  float normal[3];
};
#pragma pack(pop)

#include "model/sm.c"

// TODO: daz no good...
static struct vk_state vulkan;
static struct sln_resources resources;

/**
 * @brief Load the shaders for the game
 * 
 */
void sln_load_shaders(void) {

  // Default shader
  struct sln_file vertex_file = sln_read_file("shader-v.spv", 4);
  struct sln_file fragment_file = sln_read_file("shader-f.spv", 4);

  resources.shader = vk_create_shader(&vulkan, vertex_file.data,
    vertex_file.allocated_size, fragment_file.data,
    fragment_file.allocated_size);
    
  sln_close_file(vertex_file);
  sln_close_file(fragment_file);
}

/**
 * @brief Load the models for the game
 * 
 */
void sln_load_models(void) {

  resources.model = sln_load_sm(vulkan, "cube.sm");
}

/**
 * @brief Initialise the game
 * 
 * @param surface OS Vulkan surface
 */
void sln_init(struct vk_surface surface) {

  vulkan = vk_init(surface);
  sln_load_shaders();
  sln_load_models();
}

/**
 * @brief Call once per frame to update the game and render
 * 
 * @param app Game information
 */
void sln_update(struct sln_app app) {

  vk_render_begin(&vulkan, (float[4]){1, 1, 1, 1});
  vk_render_bind_shader(vulkan, resources.shader);
  vk_render_set_viewport(vulkan, app.width, app.height);

  VkDeviceSize offsets[1] = {0};

  vkCmdBindVertexBuffers(vulkan.command_buffer, 0, 1,
    &resources.model.vertex_buffer.buffer, offsets);

  vkCmdBindIndexBuffer(vulkan.command_buffer,
    resources.model.index_buffer.buffer.buffer, 0, VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(vulkan.command_buffer, resources.model.index_buffer.index_count,
    1, 0, 0, 0);

  vk_render_end(vulkan);
}
