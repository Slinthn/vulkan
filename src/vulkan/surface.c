/**
 * @brief Initialise the Vulkan suface. This function invokes the corresponding
 *     surface handler for the Operating System
 * 
 * @param instance Vulkan instance
 * @param appsurface App surface from OS specific code
 * @return VkSurfaceKHR New Vulkan surface
 */
VkSurfaceKHR vk_initialise_surface(
    VkInstance instance,
    struct graphics_surface appsurface
){
#ifdef SLN_WIN64
    return vk_win64(instance, appsurface);
#elif defined(SLN_X11)
    return vk_x11(instance, appsurface);
#else
    #error "No Vulkan surface has been selected."
#endif
}

/**
 * @brief Select a suitable surface format to render on
 * 
 * @param physical_device Vulkan physical device
 * @param surface Vulkan surface
 * @return VkSurfaceFormatKHR Selected Vulkan surface format
 */
VkSurfaceFormatKHR vk_select_suitable_surface_format(
    VkPhysicalDevice physical_device,
    VkSurfaceKHR surface
){
    VkSurfaceFormatKHR sf = {0};
    uint32_t sfc;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &sfc, 0);
    if (sfc == 0)
        FATAL_ERROR("No surface formats found!");

    VkSurfaceFormatKHR *sfs = malloc(sfc * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &sfc, sfs);

    sf = sfs[0];

    for (uint32_t i = 0; i < sfc; i++) {
        VkSurfaceFormatKHR format_check = sfs[i];
        uint8_t colour_check = format_check.format == VK_FORMAT_R8G8B8A8_SRGB
            && format_check.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        if (colour_check) {
            sf = format_check;
            break;
        }
    }

    free(sfs);
    return sf;
}

/**
 * @brief Calculate a suitable width and height for viewport and scissors
 * 
 * @param pd Vulkan physical device
 * @param surface Vulkan surface
 * @return VkExtent2D New Vulkan extent (width and height)
 */
VkExtent2D vk_calculate_extent(
    VkPhysicalDevice pd,
    VkSurfaceKHR surface
){
    VkExtent2D extent;
    VkSurfaceCapabilitiesKHR surface_caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface, &surface_caps);

    extent.width = min(surface_caps.maxImageExtent.width,
            max(VK_FRAMEBUFFER_WIDTH, surface_caps.minImageExtent.width));

    extent.height = min(surface_caps.maxImageExtent.height,
            max(VK_FRAMEBUFFER_HEIGHT, surface_caps.minImageExtent.height));

    return extent;
}
