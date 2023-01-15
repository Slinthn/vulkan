/**
 * @brief Main game code 
 *  
 */

#define SLN_WINDOW_WIDTH 1280
#define SLN_WINDOW_HEIGHT 720

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

struct vk_shader {
  VkPipeline pipeline;
};

#define SLN_FRAMEBUFFER_COUNT 2

struct sln_vulkan_framebuffer {
  VkImageView view;
  VkFramebuffer framebuffer;
};

struct sln_resources {
  struct vk_shader shader;
};

struct sln_vulkan_state {
  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkDevice device;
  
  union vk_queue_family queue_family;
  VkSemaphore image_ready_semaphore, render_ready_semaphore;
  VkFence render_ready_fence;
  VkExtent2D extent;
  VkCommandPool command_pool;
  VkCommandBuffer command_buffer;
  VkQueue graphics_queue, present_queue;
  VkSwapchainKHR swapchain;
  VkRenderPass render_pass;
  VkSurfaceKHR surface;
  struct sln_vulkan_framebuffer framebuffers[SLN_FRAMEBUFFER_COUNT];
  uint32_t current_image_index;
  uint32_t unused0;

#ifdef SLN_DEBUG
  VkDebugUtilsMessengerEXT debug_messenger;
#endif

  struct sln_resources resource;
};

struct vk_initialise_info {
  VkExtent2D extent;
  VkSurfaceFormatKHR format;
  struct vk_surface surface;
  uint32_t vulkan_version;
  uint32_t framebuffer_count;
};

#include "graphics/vulkan.c"
#include "file.c"

static struct sln_vulkan_state vulkan;

void sln_init(struct vk_surface surface) {

  struct vk_initialise_info init_info = {0};
  init_info.extent.width = SLN_WINDOW_WIDTH;
  init_info.extent.height = SLN_WINDOW_HEIGHT;
  init_info.format.format = VK_FORMAT_R8G8B8A8_SRGB;
  init_info.format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
  init_info.vulkan_version = VK_API_VERSION_1_0;
  init_info.surface = surface;
  init_info.framebuffer_count = 2;

  vulkan = vk_init(init_info);

  struct sln_file vertex_file = sln_read_file("shader-v.spv", 4);
  struct sln_file fragment_file = sln_read_file("shader-f.spv", 4);

  vulkan.resource.shader = vk_create_shader(vulkan.device, vulkan.render_pass,
    vertex_file.data, vertex_file.allocated_size, fragment_file.data,
    fragment_file.allocated_size);
    
  sln_close_file(vertex_file);
  sln_close_file(fragment_file);
}

void sln_update(struct sln_app app) {

  vk_render_begin(vulkan, (float[4]){1, 1, 1, 1}, &vulkan.current_image_index);

  vk_render_bind_shader(vulkan, vulkan.resource.shader);

  vk_render_set_viewport(vulkan, app.width, app.height);

  vkCmdDraw(vulkan.command_buffer, 3, 1, 0, 0);

  vk_render_end(vulkan);
}
