#ifdef SLN_VULKAN

#define VK_QUEUE_COUNT 2

union vk_queue_family {
    struct {
        uint32_t graphics, present;
    } type;
    uint32_t families[VK_QUEUE_COUNT];
};

union vk_queue {
    struct {
        VkQueue graphics, present;
    } type;
    VkQueue queues[VK_QUEUE_COUNT];
};

struct vk_framebuffer {
    VkImageView view;
    VkFramebuffer framebuffer;
};

struct vk_buffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
};

struct vk_index_buffer {
    struct vk_buffer buffer;
    uint32_t index_count;
    uint32_t unused;
};

struct vk_uniform_buffer {
    struct vk_buffer buffer;
    void *data_ptr;
    uint32_t data_size;
    uint32_t unused;
};

struct vk_shader {
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    VkDescriptorSet descriptor_set;
    struct vk_uniform_buffer uniform_buffer0, uniform_buffer1;
};

#pragma pack(push, 1)
struct vk_uniform_buffer0 {
    union matrix4 projection, view;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct vk_uniform_buffer1 {
    union matrix4 model[100];
};
#pragma pack(pop)


#pragma pack(push, 1)
struct vk_push_constant0 {
    uint32_t index;
};
#pragma pack(pop)

struct vk_push_contant0_list {
    struct vk_push_constant0 constants[100];
};

struct vk_image {
    VkImage image;
    VkDeviceMemory memory;
};

struct vk_state {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surface_format;
    union vk_queue_family queue_family;
    union vk_queue queue;
    VkSemaphore image_ready_semaphore, render_ready_semaphore;
    VkFence render_ready_fence;
    VkExtent2D extent;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkSwapchainKHR swapchain;
    VkRenderPass render_pass;
    struct vk_framebuffer framebuffers[SLN_FRAMEBUFFER_COUNT];
    struct vk_shader shader;
    uint32_t current_image_index;
    uint32_t unused0;

#ifdef SLN_DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
};

char *vk_extensions[] = {
    "VK_KHR_surface",
#ifdef SLN_DEBUG
    "VK_EXT_debug_utils",
#endif  // SLN_DEBUG
#ifdef SLN_WIN64
    "VK_KHR_win32_surface"
#endif  // SLN_WIN64
};

#ifdef SLN_WIN64
#include "vulkan_win64.c"
#endif  // SLN_WIN64

#include "vulkan_buffer.c"
#include "vulkan_render.c"
#include "vulkan_shader.c"

#ifdef SLN_DEBUG
#include "vulkan_debug.c"
#endif  // SLN_DEBUG

#include "vulkan_image.c"
#include "vulkan.c"

#endif  // SLN_VULKAN
