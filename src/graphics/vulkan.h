union vk_queue_family {
  struct {
    uint32_t graphics;
    uint32_t present;
  } type;
  uint32_t families[2];
};

struct vk_vertex {
  float position[2];
  float color[3];
};

struct vk_shader {
  VkPipeline pipeline;
};

struct vk_framebuffer {
  VkImageView view;
  VkFramebuffer framebuffer;
};

struct vk_state {
  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkDevice device;
  VkSurfaceKHR surface;
  VkSurfaceFormatKHR surface_format;
  union vk_queue_family queue_family;
  VkSemaphore image_ready_semaphore, render_ready_semaphore;
  VkFence render_ready_fence;
  VkExtent2D extent;
  VkCommandPool command_pool;
  VkCommandBuffer command_buffer;
  VkQueue graphics_queue, present_queue;
  VkSwapchainKHR swapchain;
  VkRenderPass render_pass;
  struct vk_framebuffer framebuffers[SLN_FRAMEBUFFER_COUNT];
  uint32_t current_image_index;
  uint32_t unused0;

#ifdef SLN_DEBUG
  VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

struct vk_buffer {
  VkBuffer buffer;
  VkDeviceMemory memory;
};
