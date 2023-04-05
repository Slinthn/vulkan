#define SLN_WINDOW_WIDTH 1920
#define SLN_WINDOW_HEIGHT 1080
#define SLN_FRAMEBUFFER_COUNT 2

#ifdef SLN_VULKAN
#pragma warning(push, 0)
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#pragma warning(pop)
#endif  // SLN_VULKAN

#include "math/math.h"
#include "macros.c"
#include "graphics/vulkan.c"
#include "file.c"

struct sln_model {
  struct vk_buffer vertex_buffer;
  struct vk_index_buffer index_buffer;
};

struct sln_resources {
  struct vk_shader shader;
  struct sln_model model, model2;
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

  resources.model = sln_load_sm(vulkan, "tower.sm");
  resources.model2 = sln_load_sm(vulkan, "cube.sm");
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
 * @brief TODO:
 * 
 */
void sln_draw_model(struct sln_model model) {

  VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(vulkan.command_buffer, 0, 1,
    &model.vertex_buffer.buffer, offsets);

  vkCmdBindIndexBuffer(vulkan.command_buffer, model.index_buffer.buffer.buffer,
    0, VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(vulkan.command_buffer, model.index_buffer.index_count, 1,
    0, 0, 0);
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

  static struct transform model = {0};
  model.position.c.y = sinf(model.rotation.c.y * 3) / 3.0f;
  model.rotation.c.y += 0.1f;
  model.rotation.c.z += 0.05f;
  model.rotation.c.x += 0.02f;
  model.scale = (union vector3){1, 1, 1};

  struct transform view = {0};
  view.position = (union vector3){0, 0, -4};
  view.scale = (union vector3){1, 1, 1};

  struct vk_uniform_buffer0 buf = {0};
  mat4_perspective(&buf.projection, SLN_WINDOW_HEIGHT / (float)SLN_WINDOW_WIDTH,
    DEG_TO_RAD(90), 0.1f, 10.0f);

  mat4_transform(&buf.view, view);
  mat4_transform(&buf.model, model);

  vk_update_uniform_buffer(resources.shader.uniform_buffer, &buf);

  vkCmdBindDescriptorSets(vulkan.command_buffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS, resources.shader.pipeline_layout, 0,
    1, &resources.shader.descriptor_set, 0, 0);  // TODO: double-buffering

  sln_draw_model(resources.model);
  sln_draw_model(resources.model2);

  vk_render_end(vulkan);
}
