/**
 * @brief Initialises Vulkan. Should be called after program starts. Creates
 *     everything in order to begin a Vulkan application
 * 
 * @param surface Applicaton surface
 * @return struct graphics_state A structure containing Vulkan information.
 */
struct graphics_state graphics_init(
    struct graphics_surface surface
){
    struct graphics_state s = {0};

    vk_create_instance(&s.instance);

#ifdef SLN_DEBUG
    vk_create_debug_messenger(s.instance, &s.debug_messenger);
#endif

    vk_select_suitable_physical_device(s.instance, &s.physical_device);
    vk_initialise_surface(s.instance, surface, &s.surface);
    vk_select_suitable_queue_families(s.physical_device, s.surface,
        &s.queue_family);

    vk_create_device(s.physical_device, s.queue_family, &s.device);
    vk_get_queues(s.device, s.queue_family, &s.queue);
    vk_select_suitable_surface_format(s.physical_device, s.surface,
        &s.surface_format);

    vk_calculate_extent(s.physical_device, s.surface, &s.extent);
    vk_create_swapchain(s.device, s.surface, s.surface_format, s.extent,
        s.queue_family, &s.swapchain);

    vk_create_main_render_pass(s.device, s.surface_format, &s.render_pass);
    vk_create_command_pool(s.device, s.queue_family, &s.command_pool);
    vk_create_command_buffer(s.device, s.command_pool, &s.command_buffer);

    s.depth_image = vk_create_depth_buffer(s.device, s.extent,
        s.physical_device);
    s.depth_view = vk_get_image_view(s.device, s.depth_image.image,
        VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);
    vk_get_swapchain_images(s.device, s.swapchain, s.surface_format, s.extent,
        s.render_pass, s.depth_view, s.framebuffers);

    vk_create_semaphore(s.device, &s.image_ready_semaphore);
    vk_create_semaphore(s.device, &s.render_ready_semaphore);
    vk_create_fence(s.device, &s.render_ready_fence);

    s.sampler = vk_create_nearest_sampler(s.device);

    vk_create_descriptor_pool(s.device, &s.pool);

    vk_create_descriptor_set_layout0(s.device, &s.set_layout[0]);
    vk_create_descriptor_set_layout1(s.device, &s.set_layout[1]);
    s.set_layout[2] = s.set_layout[1];

    vk_allocate_descriptor_sets(s.device, s.pool, &s.set_layout[0],
        1, &s.descriptor_set);

    s.uniform_buffer0 = vk_create_uniform_buffer(s.device, s.physical_device,
        sizeof(struct vk_uniform_buffer0));

    s.uniform_buffer1 = vk_create_uniform_buffer(s.device, s.physical_device,
        sizeof(struct vk_uniform_buffer1));

    vk_update_descriptor_set0(s.device, s.uniform_buffer0.buffer.buffer,
        s.uniform_buffer1.buffer.buffer, s.descriptor_set);

    vk_create_pipeline_layout(s.device, s.set_layout,
        SIZEOF_ARRAY(s.set_layout), &s.pipeline_layout);

    // Load shader
    struct sln_file vertex_file = sln_read_file("shader-v.spv", 4);
    struct sln_file fragment_file = sln_read_file("shader-f.spv", 4);
    struct sln_file shadow_vertex_file = sln_read_file("shadow-v.spv", 4);
    struct sln_file shadow_fragment_file = sln_read_file("shadow-f.spv", 4);

    s.shader = vk_create_shader(s.device, s.render_pass, vertex_file.data,
        vertex_file.allocated_size, fragment_file.data,
        fragment_file.allocated_size, s.pipeline_layout, VK_CULL_MODE_BACK_BIT);

    // TODO: tmp shadows
    vk_create_shadow_render_pass(s.device, &s.shadow_render_pass);

    s.shadow_image = vk_create_shadow_depth_buffer(s.device,
        s.physical_device);

    VkImageView depth_view = vk_get_image_view(s.device, s.shadow_image.image,
        VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);

    VkExtent2D ex = {VK_SHADOW_WIDTH, VK_SHADOW_HEIGHT};

    vk_create_framebuffer(s.device, ex, s.shadow_render_pass,
        1, &depth_view, &s.shadow_framebuffer);

    s.shadow_pipeline = vk_create_shader(s.device, s.shadow_render_pass,
        shadow_vertex_file.data,
        shadow_vertex_file.allocated_size, shadow_fragment_file.data,
        shadow_fragment_file.allocated_size, s.pipeline_layout,
        VK_CULL_MODE_FRONT_BIT).pipeline;

    vk_allocate_descriptor_sets(s.device, s.pool, &s.set_layout[2], 1,
        &s.shadow_set);


    s.shadow_sampler = vk_create_linear_sampler(s.device);

    vk_update_descriptor_set1(s.device, s.shadow_sampler, depth_view, s.shadow_set);

    sln_close_file(vertex_file);
    sln_close_file(fragment_file);
    sln_close_file(shadow_fragment_file);

    return s;
}

/**
 * @brief TODO:
 * 
 * @param state 
 * @param width 
 * @param height 
 */
void graphics_resize(
    struct graphics_state *s
){
    VkExtent2D extent;
    vk_calculate_extent(s->physical_device, s->surface, &extent);

    if (s->extent.width == extent.width && s->extent.height == extent.height)
        return;
    
    s->extent = extent;

    vkDeviceWaitIdle(s->device);
    vkDestroyFramebuffer(s->device, s->framebuffers[0].framebuffer, 0);
    vkDestroyFramebuffer(s->device, s->framebuffers[1].framebuffer, 0);
    vkDestroyImageView(s->device, s->framebuffers[0].view, 0);
    vkDestroyImageView(s->device, s->framebuffers[1].view, 0);
    vkDestroyImageView(s->device, s->depth_view, 0);
    vkDestroyImage(s->device, s->depth_image.image, 0);
    vkFreeMemory(s->device, s->depth_image.memory, 0);

    vkDestroySwapchainKHR(s->device, s->swapchain, 0);

    vk_create_swapchain(s->device, s->surface, s->surface_format, s->extent,
        s->queue_family, &s->swapchain);

    s->depth_image = vk_create_depth_buffer(s->device, s->extent,
        s->physical_device);
    s->depth_view = vk_get_image_view(s->device, s->depth_image.image,
        VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT);
    vk_get_swapchain_images(s->device, s->swapchain, s->surface_format,
        s->extent, s->render_pass, s->depth_view, s->framebuffers);
}
