/**
 * @brief Vulkan code
 * 
 */

/**
 * @brief Vulkan debug messages. Should not be called directly
 * 
 */
VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
  VkDebugUtilsMessageSeverityFlagBitsEXT severity,
  VkDebugUtilsMessageTypeFlagsEXT type,
  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
  void *user_data) {

  OutputDebugString(callback_data->pMessage);
  OutputDebugString("\n");

  return VK_FALSE;
}

/**
 * @brief 
 * 
 */
VkDebugUtilsMessengerCreateInfoEXT vk_populate_debug_struct(void) {

  VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

  create_info.messageType =
    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

  create_info.pfnUserCallback = vk_debug_callback;

  return create_info;
}

/**
 * @brief Creates a Vulkan instance
 * 
 * @param app_version Version of Vulkan to use
 * @param instance Returns the created instance
 * @return VkResult Vulkan errors
 */
VkResult vk_create_instance(uint32_t app_version, VkInstance *instance) {

  VkApplicationInfo app_info = {0};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.apiVersion = app_version;

  // TODO only debug for validation layers
  char *layers[] = {
    "VK_LAYER_KHRONOS_validation"
  };

  char *extensions[] = {
    "VK_EXT_debug_utils"
  };

  VkDebugUtilsMessengerCreateInfoEXT messenger_create_info =
    vk_populate_debug_struct();

  VkInstanceCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pNext = &messenger_create_info;
  create_info.pApplicationInfo = &app_info;
  create_info.enabledLayerCount = SIZEOF_ARRAY(layers);
  create_info.ppEnabledLayerNames = layers;
  create_info.enabledExtensionCount = SIZEOF_ARRAY(extensions);
  create_info.ppEnabledExtensionNames = extensions;

  return vkCreateInstance(&create_info, 0, instance);
}

/**
 * @brief Initialises the debug messaging function
 * 
 * @param instance A valid Vulkan instance
 * @param debug_messenger Returns the created debug messenger
 * @return VkResult Vulkan errors
 */
VkResult vk_create_debug_messenger(VkInstance instance,
  VkDebugUtilsMessengerEXT *debug_messenger) {

  VkDebugUtilsMessengerCreateInfoEXT create_info = vk_populate_debug_struct();

  // Extension function not loaded automatically
  PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
    (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
    "vkCreateDebugUtilsMessengerEXT");

  return vkCreateDebugUtilsMessengerEXT(instance, &create_info,
    0, debug_messenger);
}

/**
 * @brief Initialises Vulkan. Should be called after program starts
 * 
 */
void vk_init(void) {

  VkInstance instance;
  if (vk_create_instance(VK_API_VERSION_1_0, &instance) != VK_SUCCESS)
    DebugBreak();  // TODO: Better error handling

  VkDebugUtilsMessengerEXT debug_messenger;
  if (vk_create_debug_messenger(instance, &debug_messenger) != VK_SUCCESS)
    DebugBreak();  // TODO: Better error handling
}
