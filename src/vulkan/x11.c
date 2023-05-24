struct graphics_surface {
    xcb_connection_t *connection;
    xcb_window_t window;
};

/**
 * @brief Link Vulkan to an X11 instance
 * 
 * @param instance Vulkan instance
 * @param appsurface Application surface
 * @return VkSurfaceKHR New Vulkan surface
 */
VkSurfaceKHR vk_x11(
    VkInstance instance,
    struct graphics_surface appsurface
){
    VkXcbSurfaceCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    create_info.connection = appsurface.connection;
    create_info.window = appsurface.window;

    VkSurfaceKHR surface;
    vkCreateXcbSurfaceKHR(instance, &create_info, 0, &surface);
    return surface;
}
