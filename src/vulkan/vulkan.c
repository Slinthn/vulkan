/**
 * @brief Creates a Vulkan 1.0 instance, with surface support and debug
 *     validation, if SLN_DEBUG is defined
 *
 * @param instance Pointer to vulkan instance handle in which the resulting
 *     instance is returned
 */
void _vk_create_instance(
    OUT VkInstance *instance
){
    char *vk_extensions[] = {
        "VK_KHR_surface",
#ifdef SLN_DEBUG
        "VK_EXT_debug_utils",
#endif  // SLN_DEBUG
#ifdef SLN_WIN64
        "VK_KHR_win32_surface"
#endif  // SLN_WIN64
    };

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = SIZEOF_ARRAY(vk_extensions);
    create_info.ppEnabledExtensionNames = vk_extensions;

#ifdef SLN_DEBUG
    char *layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    create_info.enabledLayerCount = SIZEOF_ARRAY(layers);
    create_info.ppEnabledLayerNames = layers;

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
void _vk_select_suitable_physical_device(
    VkInstance instance,
    OUT VkPhysicalDevice *physical_device
){
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, 0);
    if (device_count == 0)
        FATAL_ERROR("No Vulkan devices detected!");

    VkPhysicalDevice *devices = malloc(device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &device_count, devices);

    *physical_device = devices[0];

    for (uint32_t i = 0; i < device_count; i++) {
        VkPhysicalDevice device_check = devices[i];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device_check, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            *physical_device = device_check;
            break;
        }
    }

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
void _vk_initialise_surface(
    VkInstance instance,
    struct vk_surface appsurface,
    OUT VkSurfaceKHR *surface
){
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
void _vk_select_suitable_queue_families(
    VkPhysicalDevice pd,
    VkSurfaceKHR surface,
    OUT union vk_queue_family *queue_family
){
    uint32_t family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &family_count, 0);
    if (family_count == 0)
        FATAL_ERROR("No queue families found on device!");

    uint64_t size = family_count * sizeof(VkQueueFamilyProperties);
    VkQueueFamilyProperties *properties = malloc(size);
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &family_count, properties);

    for (uint32_t i = 0; i < family_count; i++) {
        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queue_family->type.graphics = i;

        VkBool32 present_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, surface, &present_support);
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
void _vk_create_device(
    VkPhysicalDevice pd,
    union vk_queue_family qf,
    OUT VkDevice *device
){
    uint32_t size = SIZEOF_ARRAY(qf.families) * sizeof(VkDeviceQueueCreateInfo);
    VkDeviceQueueCreateInfo *queue_info = calloc(1, size);

    float queue_priority = 1;

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

    vkCreateDevice(pd, &create_info, 0, device);

    free(queue_info);
}

/**
 * @brief Get the command queues from a device using given queue families
 * 
 * @param device Vulkan device
 * @param family Queue families to query
 * @param queues Union of command queues in which queues will be returned in
 */
void _vk_get_queues(
    VkDevice device,
    union vk_queue_family family,
    OUT union vk_queue *queues
){
    for (uint32_t i = 0; i < VK_QUEUE_COUNT; i++)
        vkGetDeviceQueue(device, family.families[i], 0, &queues->queues[i]);
}

/**
 * @brief Select a suitable surface format to render on
 * 
 * @param pd Vulkan physical device
 * @param surface Vulkan surface
 * @param sf Handle to the surface format in which the chosen
 *     format is returned 
 */
void _vk_select_suitable_surface_format(
    VkPhysicalDevice pd,
    VkSurfaceKHR surface,
    OUT VkSurfaceFormatKHR *sf
){
    uint32_t sfc; 
    vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &sfc, 0);
    if (sfc == 0)
        FATAL_ERROR("No surface formats found!");

    VkSurfaceFormatKHR *sfs = malloc(sfc * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &sfc, sfs);

    *sf = sfs[0];

    for (uint32_t i = 0; i < sfc; i++) {
        VkSurfaceFormatKHR format_check = sfs[i];
        uint8_t colour_check = format_check.format == VK_FORMAT_R8G8B8A8_SRGB
            && format_check.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        if (colour_check) {
            *sf = format_check;
            break;
        }
    }

    free(sfs);
}

/**
 * @brief Calculate a suitable width and height for the viewport and scissors
 *  TODO:
 * @param extent Returns the extent details
 */
void _vk_calculate_extent(
    VkPhysicalDevice pd,
    VkSurfaceKHR surface,
    OUT VkExtent2D *extent
){
    VkSurfaceCapabilitiesKHR surface_caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface, &surface_caps);

    extent->width = min(surface_caps.maxImageExtent.width,
            max(VK_FRAMEBUFFER_WIDTH, surface_caps.minImageExtent.width));

    extent->height = min(surface_caps.maxImageExtent.height,
            max(VK_FRAMEBUFFER_HEIGHT, surface_caps.minImageExtent.height));
}

/**
 * @brief Create a swapchain
 * 
 * @param state Vulkan state
 * @param surface Vulkan surface
 * @param surface_format Vulkan surface format
 * @param extent Vulkan extent
 * @param families Queue families to use
 * @param swapchain Returns the created swapchain
 */
void _vk_create_swapchain(
    VkDevice device,
    VkSurfaceKHR surface,
    VkSurfaceFormatKHR surface_format,
    VkExtent2D extent,
    union vk_queue_family families,
    OUT VkSwapchainKHR *swapchain
){
    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = SLN_FRAMEBUFFER_COUNT;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t fams[] = {
        families.type.graphics,
        families.type.present
    };

    if (fams[0] == fams[1]) {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.pQueueFamilyIndices = fams;
        create_info.queueFamilyIndexCount = SIZEOF_ARRAY(fams);
    }

    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.clipped = 1;

    vkCreateSwapchainKHR(device, &create_info, 0, swapchain);
}

/**
 * @brief Create a Vulkan render pass
 * 
 * @param device Vulkan device
 * @param surface_format Vulkan surface format
 * @param render_pass Returns the created render pass
 */
void _vk_create_render_pass(
    VkDevice device,
    VkSurfaceFormatKHR surface_format,
    OUT VkRenderPass *render_pass
){
    VkAttachmentDescription att[2] = {0};
    
    // Colour
    att[0].format = surface_format.format;
    att[0].samples = VK_SAMPLE_COUNT_1_BIT;
    att[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    att[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    att[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

     // Depth stencil
    att[1].format = VK_FORMAT_D32_SFLOAT;  // TODO: check capabilities
    att[1].samples = VK_SAMPLE_COUNT_1_BIT;
    att[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    att[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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

    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = SIZEOF_ARRAY(att);
    create_info.pAttachments = att;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;

    vkCreateRenderPass(device, &create_info, 0, render_pass);
}

/**
 * @brief Create a Vulkan command pool
 * 
 * @param device Vulkan device
 * @param families Queue families
 * @param command_pool Returns the created command pool
 */
void _vk_create_command_pool(
    VkDevice device,
    union vk_queue_family families,
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
 * @param device Vulkan device
 * @param command_pool Vulkan command pool
 * @param command_buffer Returns the created command buffer
 */
void _vk_create_command_buffer( // TODO: transfer command buffer??
    VkDevice device,
    VkCommandPool command_pool,
    VkCommandBuffer *command_buffer
){
    VkCommandBufferAllocateInfo allocate_info = {0};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;

    vkAllocateCommandBuffers(device, &allocate_info, command_buffer);
}

/**
 * @brief Create a Vulkan framebuffer
 * 
 * @param device Vulkan device
 * @param extent Dimensions of framebuffer
 * @param render_pass Render pass
 * @param colour_view Framebuffer view
 * @param depth_view Depth buffer view
 * @param framebuffer Returns the framebuffer
 */
void _vk_create_framebuffer(
    VkDevice device,
    VkExtent2D extent,
    VkRenderPass render_pass,
    VkImageView colour_view,
    VkImageView depth_view,
    OUT VkFramebuffer *framebuffer
){
    VkImageView view[] = {
        colour_view,
        depth_view
    };

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
 * @brief Create a depth buffer
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @return struct vk_image 
 */
struct vk_image _vk_create_depth_buffer(
    VkDevice device,
    VkPhysicalDevice pd
){
    return vk_create_image(device, pd, VK_FORMAT_D32_SFLOAT,
        VK_FRAMEBUFFER_WIDTH, VK_FRAMEBUFFER_HEIGHT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

/**
 * @brief Creates views of all framebuffers
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param swapchain Vulkan swapchain
 * @param surface_format Vulkan surface format
 * @param extent Width and height
 * @param render_pass Vulkan render pass
 * @param framebuffers Returns the created framebuffer information
 */
void _vk_get_swapchain_images(
    VkDevice device,
    VkPhysicalDevice pd,
    VkSwapchainKHR swapchain,
    VkSurfaceFormatKHR surface_format,
    VkExtent2D extent,
    VkRenderPass render_pass,
    struct vk_framebuffer framebuffers[SLN_FRAMEBUFFER_COUNT]
){
    uint32_t image_count;
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, 0);
    if (image_count != SLN_FRAMEBUFFER_COUNT)
        FATAL_ERROR("Could not create double framebuffer!");

    VkImage *images = malloc(image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, images);

    struct vk_image depth_image = _vk_create_depth_buffer(device, pd);

    VkImageView depth_view;
    _vk_get_image_view(device, depth_image.image, VK_FORMAT_D32_SFLOAT,
        VK_IMAGE_ASPECT_DEPTH_BIT, &depth_view);

    for (uint32_t i = 0; i < SLN_FRAMEBUFFER_COUNT; i++) {
        _vk_get_image_view(device, images[i], surface_format.format,
            VK_IMAGE_ASPECT_COLOR_BIT, &framebuffers[i].view);

        _vk_create_framebuffer(device, extent, render_pass,
            framebuffers[i].view, depth_view, &framebuffers[i].framebuffer);
    }

    free(images);
}

/**
 * @brief Create a Vulkan semaphore
 * 
 * @param device Vulkan device
 * @param semaphore Returns the created semaphore
 */
void _vk_create_semaphore(
    VkDevice device,
    OUT VkSemaphore *semaphore
){
    VkSemaphoreCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(device, &create_info, 0, semaphore);
}

/**
 * @brief Create a Vulkan fence
 * 
 * @param device Vulkan device
 * @param fence Returns the created fence
 */
void _vk_create_fence(
    VkDevice device,
    VkFence *fence
){
    VkFenceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkCreateFence(device, &create_info, 0, fence);
}

/**
 * @brief Initialises Vulkan. Should be called after program starts. Creates
 *     everything in order to begin a Vulkan application
 * 
 * @param surface Applicaton surface
 * @return struct vk_state A structure containing Vulkan information.
 */
struct vk_state vk_init(
    struct vk_surface surface
){
    struct vk_state s = {0};

    _vk_create_instance(&s.instance);

#ifdef SLN_DEBUG
    _vk_create_debug_messenger(s.instance, &s.debug_messenger);
#endif

    _vk_select_suitable_physical_device(s.instance, &s.physical_device);
    _vk_initialise_surface(s.instance, surface, &s.surface);
    _vk_select_suitable_queue_families(s.physical_device, s.surface,
        &s.queue_family);

    _vk_create_device(s.physical_device, s.queue_family, &s.device);
    _vk_get_queues(s.device, s.queue_family, &s.queue);
    _vk_select_suitable_surface_format(s.physical_device, s.surface,
        &s.surface_format);

    _vk_calculate_extent(s.physical_device, s.surface, &s.extent);
    _vk_create_swapchain(s.device, s.surface, s.surface_format, s.extent,
        s.queue_family, &s.swapchain);

    _vk_create_render_pass(s.device, s.surface_format, &s.render_pass);
    _vk_create_command_pool(s.device, s.queue_family, &s.command_pool);
    _vk_create_command_buffer(s.device, s.command_pool, &s.command_buffer);

    _vk_get_swapchain_images(s.device, s.physical_device, s.swapchain,
        s.surface_format, s.extent, s.render_pass, s.framebuffers);

    _vk_create_semaphore(s.device, &s.image_ready_semaphore);
    _vk_create_semaphore(s.device, &s.render_ready_semaphore);
    _vk_create_fence(s.device, &s.render_ready_fence);

    _vk_create_sampler(s.device, &s.sampler);

    _vk_create_descriptor_pool(s.device, &s.pool);

    _vk_create_descriptor_set_layout0(s.device, &s.set_layout[0]);
    _vk_create_descriptor_set_layout1(s.device, &s.set_layout[1]);

    _vk_allocate_descriptor_sets(s.device, s.pool, &s.set_layout[0],
        1, &s.descriptor_set);

    s.uniform_buffer0 = vk_create_uniform_buffer(s.device, s.physical_device,
        sizeof(struct vk_uniform_buffer0));

    s.uniform_buffer1 = vk_create_uniform_buffer(s.device, s.physical_device,
        sizeof(struct vk_uniform_buffer1));

    _vk_update_descriptor_set0(s.device, s.uniform_buffer0.buffer.buffer,
        s.uniform_buffer1.buffer.buffer, s.descriptor_set);

    _vk_create_pipeline_layout(s.device, s.set_layout,
        SIZEOF_ARRAY(s.set_layout), &s.pipeline_layout);

#if 0
    // Texture TODO:
    struct sln_file img = sln_read_file("image.simg", 1);

    s.texture = vk_create_texture(s.device, s.physical_device,
        s.command_pool, s.queue.type.graphics, (void *)((uint64_t)img.data + 8),
        img.size, 94, 83, s.sampler, s.pool, s.set_layout[1]);

    sln_close_file(img);
#endif

    // Load shader
    struct sln_file vertex_file = sln_read_file("shader-v.spv", 4);
    struct sln_file fragment_file = sln_read_file("shader-f.spv", 4);

    s.shader = vk_create_shader(s.device, s.render_pass, vertex_file.data,
        vertex_file.allocated_size, fragment_file.data,
        fragment_file.allocated_size, s.pipeline_layout);
        
    sln_close_file(vertex_file);
    sln_close_file(fragment_file);

    return s;
}
