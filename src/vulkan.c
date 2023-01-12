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

  // TODO: Suppress warnings
  (void)severity;
  (void)type;
  (void)user_data;

  if (severity & (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
    | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)) {
    OutputDebugString(callback_data->pMessage);
    OutputDebugString("\n");
  }

  return VK_FALSE;
}

/**
 * @brief Creates and populates a VkDebugUtilsMessengerCreateInfoEXT structure
 * 
 * @return A populated VkDebugUtilsMessengerCreateInfoEXT structure
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
    "VK_EXT_debug_utils",
    "VK_KHR_surface",
    "VK_KHR_win32_surface"
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
 * @brief Enumerates and selects a physical device
 * 
 * @param instance A valid Vulkan instance
 * @param selected_device Returns the selected physical device
 * @return VkResult Vulkan errors
 */
void vk_select_device(VkInstance instance,
  VkPhysicalDevice *selected_device) {

  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, 0);

  VkPhysicalDevice *devices =
    malloc(device_count * sizeof(VkPhysicalDevice));
    // TODO: different memory allocation method?

  vkEnumeratePhysicalDevices(instance, &device_count, devices);

  // TODO: Better checks
  VkPhysicalDevice selected = devices[0];

  *selected_device = selected;

  free(devices);
}

struct vk_graphics_families {
  uint32_t graphics;
  uint32_t present;
};

/**
 * @brief Get indicies for acceptable family queues for device
 * 
 * @param physical_device Physical device to enumerate
 * @param surface Vulkan surface for present support
 * @return Enumerated device family queue indices
 */
struct vk_graphics_families vk_get_family_queues(
  VkPhysicalDevice physical_device, VkSurfaceKHR surface) {

  uint32_t count;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, 0);

  VkQueueFamilyProperties *properties =
    malloc(count * sizeof(VkQueueFamilyProperties));
  
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, properties);

  struct vk_graphics_families family = {0};

  for (uint32_t i = 0; i < count; i++) {
    if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      family.graphics = i;

    VkBool32 present_support = 0;
    vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i,
      surface, &present_support);

    if (present_support)
      family.present = i;
  }

  free(properties);

  return family;
}

/**
 * @brief Create a Vulkan device and queue
 * 
 * @param physical_device The physical device to use to create the device
 * @param queue_families The queue families to use
 * @param device Returns the created device
 * @return VkResult Vulkan errors
 */
VkResult vk_create_device_and_queue(VkPhysicalDevice physical_device,
  struct vk_graphics_families queue_families, VkDevice *device) {

  float queue_priority = 1;

  VkDeviceQueueCreateInfo queue_info[2] = {0};
  queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info[0].queueFamilyIndex = queue_families.graphics;
  queue_info[0].queueCount = 1;
  queue_info[0].pQueuePriorities = &queue_priority;

  queue_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info[1].queueFamilyIndex = queue_families.present;
  queue_info[1].queueCount = 1;
  queue_info[1].pQueuePriorities = &queue_priority;


  VkDeviceCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount = SIZEOF_ARRAY(queue_info);
  create_info.pQueueCreateInfos = queue_info;
  create_info.pEnabledFeatures = 0;  // TODO: change?

  return vkCreateDevice(physical_device, &create_info, 0, device);
}

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
    (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance,
    "vkCreateWin32SurfaceKHR");

  return vkCreateWin32SurfaceKHR(instance, &create_info, 0, surface);
}

/**
 * @brief Initialises Vulkan. Should be called after program starts
 * 
 * @param hinstance Windows HINSTNACE
 * @param hwnd Windows HWND
 */
void vk_init(HINSTANCE hinstance, HWND hwnd) {

  VkInstance instance;
  if (vk_create_instance(VK_API_VERSION_1_0, &instance) != VK_SUCCESS)
    DebugBreak();  // TODO: Better error handling

  VkDebugUtilsMessengerEXT debug_messenger;
  if (vk_create_debug_messenger(instance, &debug_messenger) != VK_SUCCESS)
    DebugBreak();  // TODO: Better error handling

  VkPhysicalDevice physical_device;
  vk_select_device(instance, &physical_device);

  VkSurfaceKHR surface;
  if (vk_win64(instance, hinstance, hwnd, &surface) != VK_SUCCESS)
    DebugBreak();

  struct vk_graphics_families families = vk_get_family_queues(physical_device,
    surface);

  VkDevice device;
  if (vk_create_device_and_queue(physical_device, families,
    &device) != VK_SUCCESS)
    DebugBreak();  // TODO: Better error handling
  
  VkQueue graphics_queue, present_queue;
  vkGetDeviceQueue(device, families.graphics, 0, &queue);
  vkGetDeviceQueue(device, families.present, 0, &queue);
}
