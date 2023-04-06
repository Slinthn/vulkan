/**
 * @brief Creates a Vulkan 1.0 instance, with surface support and debug
 *     validation, if debug is defined
 *
 * @param instance Pointer to vulkan instance handle in which the resulting
 *     instance is returned
 */
void _vk_create_instance(OUT VkInstance *instance)
{
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {0};

#ifdef SLN_DEBUG
    char *layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    create_info.enabledLayerCount = SIZEOF_ARRAY(layers);
    create_info.ppEnabledLayerNames = layers;
#endif  // SLN_DEBUG

    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = SIZEOF_ARRAY(vk_extensions);
    create_info.ppEnabledExtensionNames = vk_extensions;

#ifdef SLN_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT messenger_create_info;
    _vk_populate_debug_struct(&messenger_create_info);

    create_info.pNext = &messenger_create_info;
#endif  // SLN_DEBUG

    vkCreateInstance(&create_info, 0, instance);
}

/**
 * @brief Enumerates all graphics cards and selects a suitable physical device
 *     for the purposes of the application
 * 
 * @param instance Vulkan instance
 * @param physical_device Pointer to a physical device handle in which the
 *     resulting physical device is returned 
 */
void _vk_select_suitable_physical_device(VkInstance instance,
        OUT VkPhysicalDevice *physical_device)
{
    // TODO: VkPhysicalDeviceLimits?
    // TODO: Depth buffer capabilities?

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, 0);
    VkPhysicalDevice *devices = malloc(device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &device_count, devices);

    for (uint32_t i = 0; i < device_count; i++) {
        VkPhysicalDevice device_check = devices[i];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device_check, &properties);

        // Prefer dedicated GPUs
        if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            continue;

        *physical_device = device_check;
        goto search_complete;
    }

    // Fallback if nothing else works
    *physical_device = devices[0];

search_complete:
    free(devices);
}

/**
 * @brief Initialise the Vulkan suface. This function invokes the corresponding
 *     surface handler for the Operating System
 * 
 * @param instance Vulkan instance
 * @param appsurface App surface from OS specific code
 * @param surface Pointer to Vulkan surface handle in which the resulting handle
 *     is returned
 */
void _vk_initialise_surface(VkInstance instance, struct vk_surface appsurface,
        OUT VkSurfaceKHR *surface)
{
#ifdef SLN_WIN64
    vk_win64(instance, appsurface, surface);
#else
    #error "No Vulkan surface has been selected."
#endif
}

/**
 * @brief Find a suitable graphics and present queue family
 * 
 * @param physical_device Vulkan physical device to enumerate
 * @param surface Vulkan surface to check
 * @param queue_family Pointer to queue family in which resulting queue family
 *     information is returned
 */
void _vk_select_suitable_queue_families(VkPhysicalDevice physical_device,
        VkSurfaceKHR surface, OUT union vk_queue_family *queue_family)
{
    uint32_t family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count, 0);
    VkQueueFamilyProperties *properties =
            malloc(family_count * sizeof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &family_count,
            properties);

    for (uint32_t i = 0; i < family_count; i++) {
        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queue_family->type.graphics = i;

        VkBool32 present_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface,
                &present_support);
        if (present_support)
            queue_family->type.present = i;
    }

    free(properties);
}

/**
 * @brief Create a Vulkan device
 *
 * @param physical_device Vulkan physical device
 * @param qf Queue families to use when creating device
 * @param device Returns the handle to the created device
 */
void _vk_create_device(VkPhysicalDevice physical_device,
        union vk_queue_family qf, OUT VkDevice *device)
{
    float queue_priority = 1;

    uint32_t total_size =
            SIZEOF_ARRAY(qf.families) * sizeof(VkDeviceQueueCreateInfo);
    VkDeviceQueueCreateInfo *queue_info = calloc(1, total_size);

    for (uint32_t i = 0; i < SIZEOF_ARRAY(qf.families); i++) {
        queue_info[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[i].queueFamilyIndex = qf.families[i];
        queue_info[i].queueCount = 1;
        queue_info[i].pQueuePriorities = &queue_priority;
    }

    char *extensions[] = {
        "VK_KHR_swapchain"
    };

    VkDeviceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = SIZEOF_ARRAY(qf.families);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = SIZEOF_ARRAY(extensions);
    create_info.ppEnabledExtensionNames = extensions;
    create_info.pEnabledFeatures = 0;

    vkCreateDevice(physical_device, &create_info, 0, device);

    free(queue_info);
}

/**
 * @brief Get the command queues from a device using given queue families
 * 
 * @param device Vulkan device
 * @param family Queue families to query
 * @param queues Union of command queues in which queues will be returned in
 */
void _vk_get_queues(VkDevice device, union vk_queue_family family,
        OUT union vk_queue *queues)
{
    for (uint32_t i = 0; i < VK_QUEUE_COUNT; i++)
        vkGetDeviceQueue(device, family.families[i], 0, &queues->queues[i]);
}

/**
 * @brief Select a suitable surface format to render on
 * 
 * @param physical_device Vulkan physical device
 * @param surface Vulkan surface
 * @param surface_format Handle to the surface format in which the chosen
 *     format is returned 
 */
void _vk_select_suitable_surface_format(VkPhysicalDevice physical_device,
        VkSurfaceKHR surface, OUT VkSurfaceFormatKHR *surface_format)
{
    uint32_t surface_format_count; 
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
            &surface_format_count, 0);

    VkSurfaceFormatKHR *surface_formats =
            malloc(surface_format_count * sizeof(VkSurfaceFormatKHR));

    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface,
            &surface_format_count, surface_formats);

    for (uint32_t i = 0; i < surface_format_count; i++) {
        VkSurfaceFormatKHR format_check = surface_formats[i];
        uint8_t colour_check = format_check.format == VK_FORMAT_R8G8B8A8_SRGB
                && format_check.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        if (colour_check) {
            *surface_format = format_check;
            goto complete;
        }
    }

    // Fallback surface format
    *surface_format = surface_formats[0];

complete:
    free(surface_formats);
}

/**
 * @brief TODO
 * 
 * @param physical_device 
 * @param surface 
 * @param extent 
 */
void _vk_calculate_extent(VkPhysicalDevice physical_device,
        VkSurfaceKHR surface, OUT VkExtent2D *extent)
{
    VkSurfaceCapabilitiesKHR surface_caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface,
            &surface_caps);

    extent->width = min(surface_caps.maxImageExtent.width,
            max(SLN_WINDOW_WIDTH, surface_caps.minImageExtent.width));

    extent->height = min(surface_caps.maxImageExtent.height,
            max(SLN_WINDOW_HEIGHT, surface_caps.minImageExtent.height));
}

/**
 * @brief Create a swapchain TODO:
 * 
 * @param state Vulkan state
 */
void _vk_create_swapchain(VkDevice device, VkSurfaceKHR surface,
        VkSurfaceFormatKHR surface_format, VkExtent2D extent,
        union vk_queue_family families, OUT VkSwapchainKHR *swapchain)
{
    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = SLN_FRAMEBUFFER_COUNT;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    if (families.type.graphics == families.type.present) {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.pQueueFamilyIndices = families.families;
        create_info.queueFamilyIndexCount = SIZEOF_ARRAY(families.families);
    }

    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.clipped = 1;

    vkCreateSwapchainKHR(device, &create_info, 0, swapchain);
}

/**
 * @brief Create a Vulkan render pass TODO:
 * 
 * @param state Vulkan state
 */
void _vk_create_render_pass(VkDevice device, VkSurfaceFormatKHR surface_format,
        OUT VkRenderPass *render_pass)
{
    VkAttachmentDescription attachment[2] = {0};
    
    // Colour
    attachment[0].format = surface_format.format;
    attachment[0].samples = 1;
    attachment[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

     // Depth stencil
    attachment[1].format = VK_FORMAT_D32_SFLOAT;  // TODO: good?
    attachment[1].samples = 1;
    attachment[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment[1].finalLayout =
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colour_reference = {0};
    colour_reference.attachment = 0;
    colour_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {0};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colour_reference;
    subpass.pDepthStencilAttachment = &depth_reference;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
            | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
            | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
            | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = SIZEOF_ARRAY(attachment);
    create_info.pAttachments = attachment;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;

    vkCreateRenderPass(device, &create_info, 0, render_pass);
}

/**
 * @brief Create a Vulkan command pool TODO:
 * 
 * @param state Vulkan state
 */
void _vk_create_command_pool(VkDevice device, union vk_queue_family families,
        VkCommandPool *command_pool)
{
    VkCommandPoolCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    create_info.queueFamilyIndex = families.type.graphics;

    vkCreateCommandPool(device, &create_info, 0, command_pool);
}

/**
 * @brief Create a Vulkan command buffer
 * 
 * @param state Vulkan state
 */
void _vk_create_command_buffer(VkDevice device, VkCommandPool command_pool,
        VkCommandBuffer *command_buffer)
{
    VkCommandBufferAllocateInfo allocate_info = {0};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;

    vkAllocateCommandBuffers(device, &allocate_info, command_buffer);
}

/**
 * @brief Creates an image view for the passed image TODO:
 * 
 * @param device Vulkan device
 * @param image Image to use
 * @param format Swapchain colour format
 * @param view Returns image view
 */
void _vk_get_image_view(VkDevice device, VkImage image, VkFormat format,
        VkImageAspectFlags flags, VkImageView *view)
{
    VkImageViewCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = flags;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.layerCount = 1;

    vkCreateImageView(device, &create_info, 0, view);
}

/**
 * @brief Create a Vulkan framebuffer
 * 
 * @param device Vulkan device
 * @param extent Dimensions of framebuffer
 * @param render_pass Render pass
 * @param image_view Image view to use
 * @param framebuffer Returns the framebuffer
 */
void _vk_create_framebuffer(VkDevice device, VkExtent2D extent,
        VkRenderPass render_pass, VkImageView colour_view,
        VkImageView depth_view,
        OUT VkFramebuffer *framebuffer)
{
    VkImageView view[] = {colour_view, depth_view};

    VkFramebufferCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = render_pass;
    create_info.attachmentCount = SIZEOF_ARRAY(view);
    create_info.pAttachments = view;
    create_info.width = extent.width;
    create_info.height = extent.height;
    create_info.layers = 1;

    vkCreateFramebuffer(device, &create_info, 0, framebuffer);
}

/**
 * @brief TODO:
 * 
 * @param state 
 * @return struct vk_image 
 */
struct vk_image _vk_create_depth_buffer(VkDevice device,
        VkPhysicalDevice physical_device)
{
    return vk_create_image(device, physical_device, VK_FORMAT_D32_SFLOAT,
            SLN_WINDOW_WIDTH, SLN_WINDOW_HEIGHT,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

/**
 * @brief Creates views of framebuffers
 * 
 * @param state Vulkan state
 */
void _vk_get_swapchain_images(VkDevice device, VkPhysicalDevice physical_device,
        VkSwapchainKHR swapchain, VkSurfaceFormatKHR surface_format,
        VkExtent2D extent, VkRenderPass render_pass,
        struct vk_framebuffer framebuffers[SLN_FRAMEBUFFER_COUNT])
{
    uint32_t image_count;
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, 0);
    VkImage *images = malloc(image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, images);

    struct vk_image depth_image = _vk_create_depth_buffer(device,
            physical_device);

    VkImageView depth_view;
    _vk_get_image_view(device, depth_image.image, VK_FORMAT_D32_SFLOAT,
            VK_IMAGE_ASPECT_DEPTH_BIT, &depth_view);

    for (uint32_t i = 0; i < image_count
        && i < SLN_FRAMEBUFFER_COUNT; i++) {
        _vk_get_image_view(device, images[i], surface_format.format,
                VK_IMAGE_ASPECT_COLOR_BIT, &framebuffers[i].view);

        _vk_create_framebuffer(device, extent,
                render_pass, framebuffers[i].view, depth_view,
                &framebuffers[i].framebuffer);
    }

    free(images);
}

/**
 * @brief Create Vulkan semaphores TODO:
 * 
 * @param state Vulkan state
 */
void _vk_create_semaphore(VkDevice device, VkSemaphore *semaphore)
{
    VkSemaphoreCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(device, &create_info, 0, semaphore);
}

/**
 * @brief Create a Vulkan fence TODO:
 * 
 * @param state Vulkan state
 */
void _vk_create_fence(VkDevice device, VkFence *fence)
{
    VkFenceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkCreateFence(device, &create_info, 0, fence);
}

/**
 * @brief Initialises Vulkan. Should be called after program starts
 * 
 * @param hinstance Windows HINSTNACE
 * @param hwnd Windows HWND
 * @param extent Preferred dimensions of framebuffer
 * @param format Swapchain colour format
 * @return struct vk_state A structure containing Vulkan details
 */
struct vk_state vk_init(struct vk_surface surface)
{
    struct vk_state state = {0};

    _vk_create_instance(&state.instance);

#ifdef SLN_DEBUG
    _vk_create_debug_messenger(state.instance, &state.debug_messenger);
#endif

    _vk_select_suitable_physical_device(state.instance, &state.physical_device);
    _vk_initialise_surface(state.instance, surface, &state.surface);
    _vk_select_suitable_queue_families(state.physical_device, state.surface,
            &state.queue_family);

    _vk_create_device(state.physical_device, state.queue_family, &state.device);
    _vk_get_queues(state.device, state.queue_family, &state.queue);
    _vk_select_suitable_surface_format(state.physical_device, state.surface,
            &state.surface_format);

    _vk_calculate_extent(state.physical_device, state.surface, &state.extent);
    _vk_create_swapchain(state.device, state.surface, state.surface_format,
            state.extent, state.queue_family, &state.swapchain);

    _vk_create_render_pass(state.device, state.surface_format,
            &state.render_pass);

    _vk_create_command_pool(state.device, state.queue_family,
            &state.command_pool);

    _vk_create_command_buffer(state.device, state.command_pool,
            &state.command_buffer);

    _vk_get_swapchain_images(state.device, state.physical_device,
            state.swapchain, state.surface_format, state.extent,
            state.render_pass, state.framebuffers);

    _vk_create_semaphore(state.device, &state.image_ready_semaphore);
    _vk_create_semaphore(state.device, &state.render_ready_semaphore);
    _vk_create_fence(state.device, &state.render_ready_fence);

    // Load shader
    struct sln_file vertex_file = sln_read_file("shader-v.spv", 4);
    struct sln_file fragment_file = sln_read_file("shader-f.spv", 4);

    state.shader = vk_create_shader(&state, vertex_file.data,
        vertex_file.allocated_size, fragment_file.data,
        fragment_file.allocated_size);
        
    sln_close_file(vertex_file);
    sln_close_file(fragment_file);

    return state;
}
