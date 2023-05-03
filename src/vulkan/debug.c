/**
 * @brief Vulkan debug message handler. Called by Vulkan when a message is to
 *     be printed to the user
 * 
 * @param severity Severity of the message
 * @param type Type of message
 * @param callback_data Data of message
 * @param user_data User-defined data (in VkDebugUtilsMessengerCreateInfoEXT)
 * @return VKAPI_ATTR Return code
 */
VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
    void *user_data
){
    // Suppress warnings
    (void)type;
    (void)user_data;

    uint32_t check_bits = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

    if (severity & check_bits) {
#ifdef SLN_WIN64
        OutputDebugString(callback_data->pMessage);
        OutputDebugString("\n");
#endif
#ifdef SLN_X11
        printf("%s\n", callback_data->pMessage);
#endif
    }

    return VK_FALSE;
}

/**
 * @brief Populate a VkDebugUtilsMessengerCreateInfoEXT structure
 * 
 * @param ci Pointer in which populated structure will be returned
 */
void vk_populate_debug_struct(
    OUT VkDebugUtilsMessengerCreateInfoEXT *ci
){
    *ci = (VkDebugUtilsMessengerCreateInfoEXT){0};
    ci->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ci->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    ci->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

    ci->pfnUserCallback = vk_debug_callback;
}

/**
 * @brief Initialise the debug messaging function of Vulkan
 * 
 * @param instance Vulkan instance
 * @param debug_messenger Return handle for the created debug messenger
 */
void vk_create_debug_messenger(
    VkInstance instance,
    OUT VkDebugUtilsMessengerEXT *debug_messenger
){
    VkDebugUtilsMessengerCreateInfoEXT create_info;
    vk_populate_debug_struct(&create_info);

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)(void *)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    vkCreateDebugUtilsMessengerEXT(instance, &create_info, 0, debug_messenger);
}
