#ifdef SLN_VULKAN

#define VK_QUEUE_COUNT 2
#define VK_POOL_SIZE 1000

#define VK_FRAMEBUFFER_WIDTH 1280
#define VK_FRAMEBUFFER_HEIGHT 720

#define VK_SHADOW_WIDTH 2000
#define VK_SHADOW_HEIGHT 2000

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
    union vector4 colour;
    struct vk_buffer vertex_buffer;
    struct vk_index_buffer index_buffer;
};

struct vk_anim {
    uint32_t bone_count;
    uint32_t keyframe_count;
    union matrix4 *bones;
};

struct vk_shader {
    VkPipeline pipeline;
};

#pragma pack(push, 1)
struct vk_uniform_buffer0 {
    union matrix4 projection, view, camera_projection, camera_view;
};
#pragma pack(pop)

#define MAX_BONES 100

#pragma pack(push, 1)
struct vk_ub_anim {
    union matrix4 bones[MAX_BONES];
};
#pragma pack(pop)

// TODO: implement these
#define OBJECT_FLAG_TEXTURED 0x1
#define OBJECT_FLAG_COLOURED 0x2

#pragma pack(push, 1)
struct vk_push_constant0 {
    union matrix4 model;
    union vector4 colour;
    uint32_t flags;
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
    union vector3 normal;
    uint8_t bones[4];
    float bone_weights[4];
};
#pragma pack(pop)

#define VK_MAX_MODELS 100
#define VK_MAX_TEXTURES VK_MAX_MODELS
#define VK_MAX_OBJECTS 1000

#define VK_FLAG_EXISTS 0x1

struct vk_object {
    uint64_t flags;
    struct vk_model *model;
    struct vk_texture *texture;
    struct transform transform;
};

struct graphics_world {
    struct vk_model models[VK_MAX_MODELS];
    struct vk_texture textures[VK_MAX_TEXTURES];
    struct vk_object objects[VK_MAX_OBJECTS];
};

struct graphics_state {
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
    struct vk_image depth_image;
    VkImageView depth_view;
    struct vk_framebuffer framebuffers[SLN_FRAMEBUFFER_COUNT];
    struct vk_shader shader, terrain_shader;
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
    struct vk_uniform_buffer uniform_buffer0, ub_anim;

    struct vk_push_contant0_list push_constant_list;

    // TODO: shadow
    struct vk_image shadow_image;
    VkDescriptorSet shadow_set;
    VkPipeline shadow_pipeline;
    VkFramebuffer shadow_framebuffer;
    VkRenderPass shadow_render_pass;
    VkSampler shadow_sampler;

    struct vk_anim anim;// TODO: tmp!!
};

#ifdef SLN_WIN64
#include "win64.c"
#endif  // SLN_WIN64

#ifdef SLN_X11
#include "x11.c"
#endif  // SLN_X11

#include "buffer.c"
#include "descriptor.c"
#include "pipeline.c"
#include "shader.c"

#ifdef SLN_DEBUG
#include "debug.c"
#endif  // SLN_DEBUG

#include "image.c"
#include "renderpass.c"
#include "device.c"
#include "swapchain.c"
#include "queue.c"
#include "surface.c"
#include "timing.c"
#include "vulkan.c"

#endif  // SLN_VULKAN
