# ..\src\file.c
## sln_read_file
 Read the contents of a file to a buffer
 * 

    struct sln_file sln_read_file(
        char *filename,
        uint64_t alignment
    );

## sln_close_file
 Delete the memory of a file
 * 

    void sln_close_file(
        struct sln_file file
    );

# ..\src\game.c
## sln_init
 Initialise the game TODO:
 * 

    void sln_init(
        struct sln_app *app,
        struct graphics_surface surface
    );

## sln_update
 Call once per frame to update the game and render
 * 

    void sln_update(
        struct sln_app *app
    );

# ..\src\macros.c
# ..\src\win64.c
## win_message_proc
 Window message handler. Called when something happens to the window
 * 

    LRESULT win_message_proc(
        HWND window,
        UINT msg,
        WPARAM wparam,
        LPARAM lparam
    );

## win_game_loop
 Game loop. Should be on a separate thread
 * 

    DWORD win_game_loop(
        void *param
    );

## WinMain
 Entrypoint for WIN64 builds
 * 

    int APIENTRY WinMain(
        HINSTANCE hinstance,
        HINSTANCE prev_hinstance,
        LPSTR cmd,
        int show
    );

# ..\src\x11.c
# ..\src\math\math.h
# ..\src\math\matrix4.c
## mat4_identity
 Set values of 4x4 matrix to its identity
 * 

    void mat4_identity(
        union matrix4 *m
    );

## mat4_perspective
 Create a perspective 4x4 matrix
 * 

    void mat4_perspective(
        union matrix4 *m,
        float aspectratio,
        float fov,
        float nearz,
        float farz
    );

## mat4_orthographic
 Create an orthographic 4x4 matrix
 * 

    void mat4_orthographic(
        union matrix4 *m,
        float left,
        float right,
        float top,
        float bottom,
        float nearz,
        float farz
    );

## mat4_transform
 Create a 4x4 transformation matrix
 * 

    void mat4_transform(
        union matrix4 *m,
        struct transform transform
    );

# ..\src\math\transform.c
## tf_set
 Replace values in transform structure with specified parameters
 * 

    void tf_set(
        struct transform *tf,
        float position[3],
        float rotation[3],
        float scale[3]
    );

# ..\src\physics\collision.c
## physics_collision_resolve
 Detect a collision between the player and an object, and also resolve
 *     this collision
 * 

    void physics_collision_resolve(
        struct point_cuboid c,
        struct point_cuboid *player
    );

# ..\src\physics\physics.c
## physics_run
 Run the physics simulation
 * 

    void physics_run(
        struct physics_world *world
    );

# ..\src\physics\physics.h
# ..\src\rawinput\dualshock4.c
## rawinput_deadzone
 Calculate deadzone of PS4 controller. Returns 0 if value is between
 *     deadzone values
 * 

    float rawinput_deadzone(
        float value
    );

## rawinput_parse_dualshock4_data
 Parse DS4 data for RawInput
 * 

    void rawinput_parse_dualshock4_data(
        struct user_controls *control,
        struct ri_ds4 *data
    );

# ..\src\rawinput\keyboard.c
## rawinput_parse_keyboard_data
 Parse keyboard data for RawInput
 * 

    void rawinput_parse_keyboard_data(
        struct user_controls *control,
        RAWINPUT *data
    );

# ..\src\rawinput\mouse.c
## rawinput_parse_mouse_data
 Parse mouse data for RawInput
 * 

    void rawinput_parse_mouse_data(
        struct user_controls *control,
        RAWINPUT *data
    );

# ..\src\rawinput\rawinput.c
## rawinput_init
 Initialise RawInput
 * 

    void rawinput_init(
        HWND window
    );

## rawinput_parse
 Parse RawInput data. Should be called when a WM_INPUT event is
 *     to be handled
 * 

    void rawinput_parse(
        struct user_controls *control,
        HRAWINPUT rawinput
    );

# ..\src\rawinput\rawinput.h
# ..\src\vulkan\buffer.c
## vk_find_suitable_memory_type
 Select a suitable memory type for a buffer's requirements
 * 

    uint32_t vk_find_suitable_memory_type(
        VkMemoryRequirements requirements,
        VkPhysicalDeviceMemoryProperties properties,
        uint32_t required_flags
    );

## vk_create_buffer
 Creates a buffer
 * 

    struct vk_buffer vk_create_buffer(
        VkDevice device,
        VkPhysicalDevice physical_device,
        uint64_t bytes,
        VkBufferUsageFlagBits usage,
        VkMemoryPropertyFlags flags
    );

## vk_create_vertex_buffer
 Create a vertex buffer
 * 

    struct vk_buffer vk_create_vertex_buffer(
        VkDevice device,
        VkPhysicalDevice physical_device,
        void *data,
        uint64_t data_size
    );

## vk_create_index_buffer
 Create a index buffer
 * 

    struct vk_index_buffer vk_create_index_buffer(
        VkDevice device,
        VkPhysicalDevice physical_device,
        uint32_t *indices,
        uint32_t index_count
    );

## vk_create_uniform_buffer
 Create a uniform buffer
 *

    struct vk_uniform_buffer vk_create_uniform_buffer(
        VkDevice device,
        VkPhysicalDevice physical_device,
        uint32_t data_size
    );

## vk_update_uniform_buffer
 Update the data in a uniform buffer. GPU sees changes immediately
 *     due to VK_MEMORY_PROPERTY_HOST_COHERENT_BIT flag
 * 

    void vk_update_uniform_buffer(
        struct vk_uniform_buffer ub,
        void *data
    );

# ..\src\vulkan\debug.c
## vk_debug_callback
 Vulkan debug message handler. Called by Vulkan when a message is to
 *     be printed to the user
 * 

    VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
        void *user_data
    );

## vk_populate_debug_struct
 Populate a VkDebugUtilsMessengerCreateInfoEXT structure
 * 

    void vk_populate_debug_struct(
        OUT VkDebugUtilsMessengerCreateInfoEXT *ci
    );

## vk_create_debug_messenger
 Initialise the debug messaging function of Vulkan
 * 

    void vk_create_debug_messenger(
        VkInstance instance,
        OUT VkDebugUtilsMessengerEXT *debug_messenger
    );

# ..\src\vulkan\descriptor.c
## vk_create_descriptor_pool
 Create a descriptor pool
 * 

    void vk_create_descriptor_pool(
        VkDevice device,
        OUT VkDescriptorPool *pool
    );

## vk_allocate_descriptor_sets
 Allocate descriptor set from a set layout TODO:
 * 

    void vk_allocate_descriptor_sets(
        VkDevice device,
        VkDescriptorPool pool,
        VkDescriptorSetLayout *set_layouts,
        uint32_t set_layout_count,
        OUT VkDescriptorSet *set
    );

## vk_update_descriptor_set0
 TODO:
 * 

    void vk_update_descriptor_set0(
        VkDevice device,
        VkBuffer buffer0,
        VkBuffer buffer1,
        VkDescriptorSet set
    );

## vk_update_descriptor_set1
 TODO:
 * 

    void vk_update_descriptor_set1(
        VkDevice device,
        VkSampler sampler,
        VkImageView image_view,
        VkDescriptorSet set
    );

# ..\src\vulkan\device.c
## vk_create_instance
 Creates a Vulkan 1.0 instance, with surface support and debug
 *     validation, if SLN_DEBUG is defined
 *

    void vk_create_instance(
        OUT VkInstance *instance
    );

## vk_select_suitable_physical_device
 Enumerates all graphics cards and selects a suitable physical device
 *     for the purposes of the application
 * 

    void vk_select_suitable_physical_device(
        VkInstance instance,
        OUT VkPhysicalDevice *physical_device
    );

## vk_create_device
 Create a Vulkan device
 *

    void vk_create_device(
        VkPhysicalDevice pd,
        union vk_queue_family qf,
        OUT VkDevice *device
    );

# ..\src\vulkan\image.c
## vk_get_image_view
 Creates an image view for the passed image
 * 

    VkImageView vk_get_image_view(
        VkDevice device,
        VkImage image,
        VkFormat format,
        VkImageAspectFlags flags
    );

## vk_create_image
 Create a 2D image
 * 

    struct vk_image vk_create_image(
        VkDevice device,
        VkPhysicalDevice physical_device,
        VkFormat format,
        uint32_t width,
        uint32_t height,
        VkImageUsageFlagBits usage,
        VkMemoryPropertyFlags flags
    );

## vk_transition_image
 Transition an image resource barrier. Must be called in between a
 *     vkBeginCommandBuffer and vEndCommandBuffer
 * 

    void vk_transition_image(
        VkCommandBuffer command_buffer,
        VkPipelineStageFlags src_stage_flags,
        VkPipelineStageFlags dst_stage_flags,
        VkAccessFlags src_access_flags,
        VkAccessFlags dst_access_flags,
        VkImageLayout old_layout,
        VkImageLayout new_layout,
        VkImageAspectFlags aspect_mask,
        VkImage image
    );

## vk_create_texture
 Create a texture
 * 

    struct vk_texture vk_create_texture(
        VkDevice device,
        VkPhysicalDevice physical_device,
        VkCommandPool command_pool,  // TODO: i hate this, a command pool and queue?
        VkQueue graphics_queue,
        void *data,
        uint64_t bytes,
        uint32_t width,
        uint32_t height,
        VkSampler sampler,
        VkDescriptorPool pool,
        VkDescriptorSetLayout set_layout
    );

## vk_create_sampler
 Create a sampler for a texture TODO:
 * 

    VkSampler vk_create_sampler(
        VkDevice device,
        VkFilter filter
    );

## vk_create_depth_buffer
 Create a depth buffer TODO:
 * 

    struct vk_image vk_create_depth_buffer(
        VkDevice device,
        VkExtent2D extent,
        VkPhysicalDevice pd
    );

## vk_create_shadow_depth_buffer
 Create a depth buffer
 * 

    struct vk_image vk_create_shadow_depth_buffer(
        VkDevice device,
        VkPhysicalDevice pd
    );

# ..\src\vulkan\pipeline.c
## vk_create_pipeline_layout
 Create a Vulkan pipeline layout TODO:
 * 

    void vk_create_pipeline_layout(
        VkDevice device,
        VkDescriptorSetLayout *set_layouts,
        uint32_t set_layout_count,
        OUT VkPipelineLayout *pipeline
    );

## vk_create_graphics_pipeline
 Creates a Vulkan graphics pipeline TODO:
 * 

    void vk_create_graphics_pipeline(
        VkDevice device,
        VkPipelineShaderStageCreateInfo vertex_stage,
        VkPipelineShaderStageCreateInfo fragment_stage,
        VkRenderPass render_pass,
        VkPipelineLayout pipeline_layout,
        VkCullModeFlags culling,
        OUT VkPipeline *pipeline
    );

# ..\src\vulkan\queue.c
## vk_select_suitable_queue_families
 Find a suitable graphics and present queue family
 * 

    void vk_select_suitable_queue_families(
        VkPhysicalDevice pd,
        VkSurfaceKHR surface,
        OUT union vk_queue_family *queue_family
    );

## vk_create_command_buffer
 Create a Vulkan command buffer
 * 

    void vk_create_command_buffer( // TODO: transfer command queue/buffer??
        VkDevice device,
        VkCommandPool command_pool,
        VkCommandBuffer *command_buffer
    );

## vk_get_queues
 Get the command queues from a device using given queue families
 * 

    void vk_get_queues(
        VkDevice device,
        union vk_queue_family family,
        OUT union vk_queue *queues
    );

# ..\src\vulkan\render.c
## vk_render_set_viewport
 Set the size of the viewport (and scissor, which is set to the same)
 * 

    void vk_render_set_viewport(
        VkCommandBuffer command_buffer,
        uint32_t width,
        uint32_t height
    );

## vk_render_begin
 Begin the render pass. Should be called before any other render
 *     operations
 * 

    int32_t vk_render_begin(
        struct graphics_state *state,
        struct vk_uniform_buffer0 *buffer0,
        struct vk_uniform_buffer1 *buffer1
    );

## vk_render_main
 Main colour rendering. Should be called after rendering to the
 *     shadow depth buffer is complete
 * 

    void vk_render_main(
        struct graphics_state *state,
        float clear_color[4],
        uint32_t viewport_width,
        uint32_t viewport_height
    );

## vk_render_shadow
 Prepare to render shadows
 * 

    void vk_render_shadow(
        struct graphics_state *state
    );

## vk_render_end
 Completes render pass. Renders image to user
 * 

    void vk_render_end(
        struct graphics_state state
    );

## sln_draw_model
 Draw a model
 * 

    void sln_draw_model(
        struct graphics_state *state,
        struct vk_model model,
        struct vk_texture texture,
        struct vk_push_constant0 *constant
    );

## graphics_render
 Render all objects TODO:
 * 

    void graphics_render(
        struct graphics_state *state,
        struct sln_app app,
        struct transform view,
        struct graphics_world world
    );

# ..\src\vulkan\renderpass.c
## vk_create_render_pass
 Create a Vulkan render pass
 * 

    void vk_create_render_pass(
        VkDevice device,
        uint32_t attachment_count,
        VkAttachmentDescription *attachments,
        VkSubpassDescription *subpass,
        OUT VkRenderPass *render_pass
    );

## vk_create_main_render_pass
 TODO:
 * 

    void vk_create_main_render_pass(
        VkDevice device,
        VkSurfaceFormatKHR surface_format,
        OUT VkRenderPass *render_pass
    );

# ..\src\vulkan\shader.c
## vk_create_shader_module
 Creates a shader module
 * 

    void vk_create_shader_module(
        VkDevice device,
        void *code,
        uint64_t bytes,
        OUT VkShaderModule *module
    );

## vk_create_shader
 Create a vertex and fragment shader into a single graphics pipeline
 * 

    struct vk_shader vk_create_shader(
        VkDevice device,
        VkRenderPass render_pass,
        void *vertex_data,
        uint64_t vertex_size,
        void *fragment_data,
        uint64_t fragment_size,
        VkPipelineLayout pipeline_layout,
        VkCullModeFlags culling
    );

# ..\src\vulkan\surface.c
## vk_initialise_surface
 Initialise the Vulkan suface. This function invokes the corresponding
 *     surface handler for the Operating System
 * 

    void vk_initialise_surface(
        VkInstance instance,
        struct graphics_surface appsurface,
        OUT VkSurfaceKHR *surface
    );

## vk_select_suitable_surface_format
 Select a suitable surface format to render on
 * 

    void vk_select_suitable_surface_format(
        VkPhysicalDevice pd,
        VkSurfaceKHR surface,
        OUT VkSurfaceFormatKHR *sf
    );

## vk_calculate_extent
 Calculate a suitable width and height for the viewport and scissors
 *  TODO:

    void vk_calculate_extent(
        VkPhysicalDevice pd,
        VkSurfaceKHR surface,
        OUT VkExtent2D *extent
    );

# ..\src\vulkan\swapchain.c
## vk_create_swapchain
 Create a swapchain
 * 

    void vk_create_swapchain(
        VkDevice device,
        VkSurfaceKHR surface,
        VkSurfaceFormatKHR surface_format,
        VkExtent2D extent,
        union vk_queue_family families,
        OUT VkSwapchainKHR *swapchain
    );

## vk_create_framebuffer
 Create a Vulkan framebuffer TODO:
 * 

    void vk_create_framebuffer(
        VkDevice device,
        VkExtent2D extent,
        VkRenderPass render_pass,
        uint32_t view_count,
        VkImageView *views,
        OUT VkFramebuffer *framebuffer
    );

## vk_get_swapchain_images
 Creates views of all framebuffers TODO:
 * 

    void vk_get_swapchain_images(
        VkDevice device,
        VkSwapchainKHR swapchain,
        VkSurfaceFormatKHR surface_format,
        VkExtent2D extent,
        VkRenderPass render_pass,
        VkImageView depth_view,
        struct vk_framebuffer framebuffers[SLN_FRAMEBUFFER_COUNT]
    );

# ..\src\vulkan\timing.c
## vk_create_semaphore
 Create a Vulkan semaphore
 * 

    void vk_create_semaphore(
        VkDevice device,
        OUT VkSemaphore *semaphore
    );

## vk_create_fence
 Create a Vulkan fence
 * 

    void vk_create_fence(
        VkDevice device,
        VkFence *fence
    );

# ..\src\vulkan\vulkan.c
## graphics_init
 Initialises Vulkan. Should be called after program starts. Creates
 *     everything in order to begin a Vulkan application
 * 

    struct graphics_state graphics_init(
        struct graphics_surface surface
    );

## graphics_resize
 TODO:
 * 

    void graphics_resize(
        struct graphics_state *s
    );

# ..\src\vulkan\vulkan.h
# ..\src\vulkan\win64.c
## vk_win64
 Link Vulkan to a Windows HWND
 * 

    void vk_win64(
        VkInstance instance,
        struct graphics_surface appsurface,
        OUT VkSurfaceKHR *surface
    );

# ..\src\vulkan\world.c
## graphics_load_sm
 Load a .sm file from disk and create buffers for Vulkan
 * 

    struct vk_model graphics_load_sm(
        VkDevice device,
        VkPhysicalDevice physical_device,
        char *filename
    );

# ..\src\vulkan\x11.c
## vk_x11
 Link Vulkan to an X11 instance
 * 

    void vk_x11(
        VkInstance instance,
        struct graphics_surface appsurface,
        OUT VkSurfaceKHR *surface
    );

# ..\src\vulkan\glsl\shader.frag
# ..\src\vulkan\glsl\shader.vert
# ..\src\vulkan\glsl\shadow.frag
# ..\src\vulkan\glsl\shadow.vert
# ..\src\world\world.c
## sln_load_sw
 Load a .sm file from disk and create buffers for Vulkan
 * 

    struct sw_world sln_load_sw(
        char *filename
    );

# ..\src\world\world.h
