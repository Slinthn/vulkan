#ifdef SLN_WIN64

/**
 * @brief Link Vulkan to a HWND
 * 
 * @param state Vulkan state
 * @param appsurface Application surface
 */
void vk_win64(struct vk_state *state, struct vk_surface appsurface) {

  VkWin32SurfaceCreateInfoKHR create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  create_info.hinstance = appsurface.hinstance;
  create_info.hwnd = appsurface.hwnd;

  // Extension function not loaded automatically
  PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR =
    (PFN_vkCreateWin32SurfaceKHR)(void *)vkGetInstanceProcAddr(state->instance,
    "vkCreateWin32SurfaceKHR");

  vkCreateWin32SurfaceKHR(state->instance, &create_info, 0, &state->surface);
}

#endif
