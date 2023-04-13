#ifdef SLN_VULKAN

#define VK_QUEUE_COUNT 2
#define VK_POOL_SIZE 1000

#define VK_FRAMEBUFFER_WIDTH 1280
#define VK_FRAMEBUFFER_HEIGHT 720

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

struct vk_model {
    struct vk_buffer vertex_buffer;
    struct vk_index_buffer index_buffer;
};

struct vk_shader {
    VkPipeline pipeline;
};

#pragma pack(push, 1)
struct vk_uniform_buffer0 {
    union matrix4 projection, view, camera_projection, camera_view;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct vk_uniform_buffer1 {
    union matrix4 model[100];  // TODO: random number
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

struct vk_texture {
    struct vk_image image;
    VkImageView image_view;
    VkDescriptorSet set;
};

#pragma pack(push, 1)
struct vk_vertex {
    union vector3 position;
    union vector2 texture;
    union vector3 normal;
};
#pragma pack(pop)

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
    VkSampler sampler;

#ifdef SLN_DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    VkDescriptorPool pool;
    VkPipelineLayout pipeline_layout;
    VkDescriptorSetLayout set_layout[3];
    VkDescriptorSet descriptor_set;
    struct vk_uniform_buffer uniform_buffer0, uniform_buffer1;
    struct vk_texture texture;

    // TODO: shadow
    struct vk_image depth_image;
    VkDescriptorSet shadow_set;
    VkPipeline shadow_pipeline;
    VkFramebuffer shadow_framebuffer;
    VkRenderPass shadow_render_pass;
};

#ifdef SLN_WIN64
#include "vulkan_win64.c"
#endif  // SLN_WIN64

#include "vulkan_buffer.c"
#include "vulkan_shader.c"

#ifdef SLN_DEBUG
#include "vulkan_debug.c"
#endif  // SLN_DEBUG

#include "vulkan_image.c"
#include "vulkan_render.c"
#include "vulkan.c"

#endif  // SLN_VULKAN
