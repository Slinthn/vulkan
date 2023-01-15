#ifdef SLN_WIN64

/**
 * @brief Link Vulkan to a HWND
 * 
 * @param instance Vulkan instance
 * @param hinstance Windows HINSTANCE
 * @param hwnd Windows HWND
 * @param surface Returns a Vulkan surface
 * @return VkResult Vulkan errors
 */
VkResult vk_win64(VkInstance instance, HINSTANCE hinstance,
  HWND hwnd, VkSurfaceKHR *surface) {

  VkWin32SurfaceCreateInfoKHR create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  create_info.hinstance = hinstance;
  create_info.hwnd = hwnd;

  // Extension function not loaded automatically
  PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR =
    (PFN_vkCreateWin32SurfaceKHR)(void *)vkGetInstanceProcAddr(instance,
    "vkCreateWin32SurfaceKHR");

  return vkCreateWin32SurfaceKHR(instance, &create_info, 0, surface);
}

#endif
