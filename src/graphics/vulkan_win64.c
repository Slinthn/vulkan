#ifdef SLN_VULKAN
#ifdef SLN_WIN64

/**
 * @brief Vulkan Windows surface. To be used for binding Vulkan to a window
 * 
 */
struct vk_surface {
  HINSTANCE hinstance;
  HWND hwnd;
};

/**
 * @brief Link Vulkan to a Windows HWND
 * 
 * @param instance Vulkan instance
 * @param appsurface Application surface
 * @param surface Pointer to Vulkan surface handle in which the resulting
 *   surface is returned
 */
void vk_win64(
  VkInstance instance,
  struct vk_surface appsurface,
  OUT VkSurfaceKHR *surface
) {
  VkWin32SurfaceCreateInfoKHR create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  create_info.hinstance = appsurface.hinstance;
  create_info.hwnd = appsurface.hwnd;

  PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR =
    (PFN_vkCreateWin32SurfaceKHR)(void *)vkGetInstanceProcAddr(instance,
    "vkCreateWin32SurfaceKHR");

  vkCreateWin32SurfaceKHR(instance, &create_info, 0, surface);
}

#endif  // SLN_WIN64
#endif  // SLN_VULKAN
