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
//#ifdef SLN_X11
        FILE *file = fopen("debug.txt", "a");
        fwrite(callback_data->pMessage, strlen(callback_data->pMessage), 1,
            file);
        fwrite("\n", 1, 1, file);
        fclose(file);
//#endif
    }

    return VK_FALSE;
}

/**
 * @brief Populate a VkDebugUtilsMessengerCreateInfoEXT structure
 * 
 * @return VkDebugUtilsMessengerCreateInfoEXT Populated structure
 */
VkDebugUtilsMessengerCreateInfoEXT vk_populate_debug_struct(
    void
){
    VkDebugUtilsMessengerCreateInfoEXT ci
        = (VkDebugUtilsMessengerCreateInfoEXT){0};
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ci.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    ci.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
        | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;

    ci.pfnUserCallback = vk_debug_callback;
    return ci;
}

/**
 * @brief Initialise the debug messaging function of Vulkan
 * 
 * @param instance Vulkan instance
 * @return VkDebugUtilsMessengerEXT Handle for the created debug messenger
 */
VkDebugUtilsMessengerEXT vk_create_debug_messenger(
    VkInstance instance
){
    VkDebugUtilsMessengerCreateInfoEXT create_info = vk_populate_debug_struct();

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)(void *)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    
    VkDebugUtilsMessengerEXT debug_messenger;
    vkCreateDebugUtilsMessengerEXT(instance, &create_info, 0, &debug_messenger);
    return debug_messenger;
}
