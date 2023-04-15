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
void vk_create_swapchain(
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
    create_info.imageExtent.width = extent.width;
    create_info.imageExtent.height = extent.height;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.clipped = 1;

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

    vkCreateSwapchainKHR(device, &create_info, 0, swapchain);
}

/**
 * @brief Create a Vulkan framebuffer TODO:
 * 
 * @param device Vulkan device
 * @param extent Dimensions of framebuffer
 * @param render_pass Render pass
 * @param colour_view Framebuffer view
 * @param depth_view Depth buffer view
 * @param framebuffer Returns the framebuffer
 */
void vk_create_framebuffer(
    VkDevice device,
    VkExtent2D extent,
    VkRenderPass render_pass,
    uint32_t view_count,
    VkImageView *views,
    OUT VkFramebuffer *framebuffer
){
    VkFramebufferCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    create_info.renderPass = render_pass;
    create_info.attachmentCount = view_count;
    create_info.pAttachments = views;
    create_info.width = extent.width;
    create_info.height = extent.height;
    create_info.layers = 1;

    vkCreateFramebuffer(device, &create_info, 0, framebuffer);
}

/**
 * @brief Creates views of all framebuffers TODO:
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param swapchain Vulkan swapchain
 * @param surface_format Vulkan surface format
 * @param extent Width and height
 * @param render_pass Vulkan render pass
 * @param framebuffers Returns the created framebuffer information
 */
void vk_get_swapchain_images(
    VkDevice device,
    VkSwapchainKHR swapchain,
    VkSurfaceFormatKHR surface_format,
    VkExtent2D extent,
    VkRenderPass render_pass,
    VkImageView depth_view,
    struct vk_framebuffer framebuffers[SLN_FRAMEBUFFER_COUNT]
){
    // TODO: this function does too much
    uint32_t image_count;
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, 0);
    if (image_count != SLN_FRAMEBUFFER_COUNT)
        FATAL_ERROR("Could not create double framebuffer!");

    VkImage *images = malloc(image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device, swapchain, &image_count, images);

    for (uint32_t i = 0; i < SLN_FRAMEBUFFER_COUNT; i++) {
        framebuffers[i].view = vk_get_image_view(device, images[i],
            surface_format.format, VK_IMAGE_ASPECT_COLOR_BIT);

        VkImageView views[2] = {
            framebuffers[i].view,
            depth_view
        };

        vk_create_framebuffer(device, extent, render_pass,
            SIZEOF_ARRAY(views), views, &framebuffers[i].framebuffer);
    }

    free(images);
}
