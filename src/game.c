/**
 * @brief Main game code 
 *  
 */

#define SLN_WINDOW_WIDTH 1280
#define SLN_WINDOW_HEIGHT 720

#include "macros.c"

#define SLN_FRAMEBUFFER_COUNT 2

struct vk_vertex {
  float position[2];
  float color[3];
};

#include "graphics/vulkan_init.c"
#include "file.c"

struct sln_resources {
  struct vk_shader shader;
  struct vk_buffer vertex_buffer;
};

static struct vk_state vulkan;
static struct sln_resources resources;


void tmp_sln_vertex(void) {

  struct vk_vertex vertices[] = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
  };

  struct vk_buffer_info buffer_info = {0};
  buffer_info.device = vulkan.device;
  buffer_info.physical_device = vulkan.physical_device;
  buffer_info.size = sizeof(vertices);
  buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  buffer_info.queue_family = vulkan.queue_family;
  buffer_info.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  resources.vertex_buffer = vk_create_buffer(buffer_info);

  uint8_t *data;
  vkMapMemory(vulkan.device, resources.vertex_buffer.memory, 0,
    VK_WHOLE_SIZE, 0, &data);

  memcpy(data, vertices, sizeof(vertices));
  vkUnmapMemory(vulkan.device, resources.vertex_buffer.memory);
}

void sln_init(struct vk_surface surface) {

  struct vk_initialise_info init_info = {0};
  init_info.extent.width = SLN_WINDOW_WIDTH;
  init_info.extent.height = SLN_WINDOW_HEIGHT;
  init_info.surface = surface;

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

  tmp_sln_vertex();
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
