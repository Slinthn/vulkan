#ifdef SLN_VULKAN
#ifdef SLN_DEBUG

#pragma warning(disable:4100)
/**
 * @brief Vulkan debug message handler. Called by Vulkan when a message is to
 *   be printed to the console
 * 
 * @param severity Severity of the message
 * @param type Type of message
 * @param callback_data Data of message
 * @param user_data User-defined data (in VkDebugUtilsMessengerCreateInfoEXT)
 * @return VKAPI_ATTR Return code
 */
VKAPI_ATTR VkBool32 VKAPI_CALL _vk_debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT severity,
  VkDebugUtilsMessageTypeFlagsEXT type,
  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
  void *user_data) {
#pragma warning(default:4100)

  if (severity & (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)) {
    OutputDebugString(callback_data->pMessage);
    OutputDebugString("\n");
  }

  return VK_FALSE;
}

/**
 * @brief Utility function to create and populate a
 *   VkDebugUtilsMessengerCreateInfoEXT structure
 * 
 * @param create_info Pointer in which populated structure will be returned
 */
void _vk_populate_debug_struct(
  VkDebugUtilsMessengerCreateInfoEXT *create_info) {

  *create_info = (VkDebugUtilsMessengerCreateInfoEXT){0};
  create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info->messageSeverity =
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

  create_info->messageType =
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

  create_info->pfnUserCallback = _vk_debug_callback;
}

/**
 * @brief Initialise the debug messaging function of Vulkan
 * 
 * @param instance Vulkan instance
 * @param debug_messenger Return handle for the created debug messenger
 */
void _vk_create_debug_messenger(VkInstance instance,
  VkDebugUtilsMessengerEXT *debug_messenger) {

  VkDebugUtilsMessengerCreateInfoEXT create_info;
  _vk_populate_debug_struct(&create_info);

  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
    (PFN_vkCreateDebugUtilsMessengerEXT)(void *)vkGetInstanceProcAddr(
    instance, "vkCreateDebugUtilsMessengerEXT");

  vkCreateDebugUtilsMessengerEXT(instance, &create_info, 0,
    debug_messenger);
}

#endif  // SLN_DEBUG
#endif  // SLN_VULKAN
