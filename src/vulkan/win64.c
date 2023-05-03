struct graphics_surface {
    HINSTANCE hinstance;
    HWND hwnd;
};

/**
 * @brief Link Vulkan to a Windows HWND
 * 
 * @param instance Vulkan instance
 * @param appsurface Application surface
 * @param surface Pointer to Vulkan surface handle in which the resulting
 *     surface is returned
 */
void vk_win64(
    VkInstance instance,
    struct graphics_surface appsurface,
    OUT VkSurfaceKHR *surface
){
    VkWin32SurfaceCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hinstance = appsurface.hinstance;
    create_info.hwnd = appsurface.hwnd;

    vkCreateWin32SurfaceKHR(instance, &create_info, 0, surface);
}

