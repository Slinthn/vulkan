struct graphics_surface {
    xcb_connection_t *connection;
    xcb_window_t window;
};

/**
 * @brief Link Vulkan to an X11 instance
 * 
 * @param instance Vulkan instance
 * @param appsurface Application surface
 * @param surface Pointer to Vulkan surface handle in which the resulting
 *     surface is returned
 */
void vk_x11(
    VkInstance instance,
    struct graphics_surface appsurface,
    OUT VkSurfaceKHR *surface
){
    VkXcbSurfaceCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    create_info.connection = appsurface.connection;
    create_info.window = appsurface.window;

    vkCreateXcbSurfaceKHR(instance, &create_info, 0, surface);
}

