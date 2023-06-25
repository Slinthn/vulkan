/**
 * @brief Creates a Vulkan 1.0 instance, with surface support and debug
 *     validation, if SLN_DEBUG is defined
 *
 * @return VkInstance New Vulkan instance
 */
VkInstance vk_create_instance(
    void
){
    char *vk_extensions[] = {
        "VK_KHR_surface",
#ifdef SLN_DEBUG
        "VK_EXT_debug_utils",
#endif  // SLN_DEBUG
#ifdef SLN_WIN64
        "VK_KHR_win32_surface",
#endif  // SLN_WIN64
#ifdef SLN_X11
        "VK_KHR_xcb_surface",
#endif  // SLN_X11
    };

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = SIZEOF_ARRAY(vk_extensions);
    create_info.ppEnabledExtensionNames = (const char *const *)vk_extensions;

#ifdef SLN_DEBUG
    char *layers[] = {
        "VK_LAYER_KHRONOS_validation"
    };

    create_info.enabledLayerCount = SIZEOF_ARRAY(layers);
    create_info.ppEnabledLayerNames = (const char *const *)layers;

    VkDebugUtilsMessengerCreateInfoEXT messenger_create_info
        = vk_populate_debug_struct();

    create_info.pNext = &messenger_create_info;
#endif  // SLN_DEBUG

    VkInstance instance;
    vkCreateInstance(&create_info, 0, &instance);
    return instance;
}

/**
 * @brief Enumerates all graphics cards and selects a suitable physical device
 *     for the purposes of the application
 * 
 * @param instance Vulkan instance
 * @return VkPhysicalDevice New Vulkan physical device
 */
VkPhysicalDevice vk_select_suitable_physical_device(
    VkInstance instance
){
    VkPhysicalDevice physical_device = 0;
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, 0);
    if (device_count == 0)
        FATAL_ERROR("No Vulkan devices detected!");

    VkPhysicalDevice *devices = malloc(device_count * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &device_count, devices);

    physical_device = devices[0];

    for (uint32_t i = 0; i < device_count; i++) {
        VkPhysicalDevice device_check = devices[i];

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device_check, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            physical_device = device_check;
            break;
        } else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            physical_device = device_check;
        }
    }

    free(devices);
    return physical_device;
}

/**
 * @brief Create a Vulkan device
 *
 * @param physical_device Vulkan physical device
 * @param qf Queue families to use when creating device
 * @return VkDevice New Vulkan device
 */
VkDevice vk_create_device(
    VkPhysicalDevice physical_device,
    union vk_queue_family qf
){
    uint32_t size = SIZEOF_ARRAY(qf.families) * sizeof(VkDeviceQueueCreateInfo);
    VkDeviceQueueCreateInfo *queue_info = calloc(1, size);

    float queue_priority = 1;

    for (uint32_t i = 0; i < SIZEOF_ARRAY(qf.families); i++) {
        queue_info[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[i].queueFamilyIndex = qf.families[i];
        queue_info[i].queueCount = 1;
        queue_info[i].pQueuePriorities = &queue_priority;
    }

    char *extensions[] = {
        "VK_KHR_swapchain"
    };

    VkDeviceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = SIZEOF_ARRAY(qf.families);
    create_info.pQueueCreateInfos = queue_info;
    create_info.enabledExtensionCount = SIZEOF_ARRAY(extensions);
    create_info.ppEnabledExtensionNames = (const char *const *)extensions;
    create_info.pEnabledFeatures = 0;

    VkDevice device;
    vkCreateDevice(physical_device, &create_info, 0, &device);

    free(queue_info);
    return device;
}
