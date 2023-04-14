/**
 * @brief Initialise the Vulkan suface. This function invokes the corresponding
 *     surface handler for the Operating System
 * 
 * @param instance Vulkan instance
 * @param appsurface App surface from OS specific code
 * @param surface Pointer to Vulkan surface handle in which the resulting handle
 *     is returned
 */
void vk_initialise_surface(
    VkInstance instance,
    struct graphics_surface appsurface,
    OUT VkSurfaceKHR *surface
){
#ifdef SLN_WIN64
    vk_win64(instance, appsurface, surface);
#else
    #error "No Vulkan surface has been selected."
#endif
}

/**
 * @brief Select a suitable surface format to render on
 * 
 * @param pd Vulkan physical device
 * @param surface Vulkan surface
 * @param sf Handle to the surface format in which the chosen
 *     format is returned 
 */
void vk_select_suitable_surface_format(
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
void vk_calculate_extent(
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
