/**
 * @brief Find a suitable graphics and present queue family
 * 
 * @param pd Vulkan physical device to enumerate
 * @param surface Vulkan surface to check
 * @return union vk_queue_family Chosen queue families
 */
union vk_queue_family vk_select_suitable_queue_families(
    VkPhysicalDevice pd,
    VkSurfaceKHR surface
){
    union vk_queue_family queue_family = {0};
    uint32_t family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &family_count, 0);
    if (family_count == 0)
        FATAL_ERROR("No queue families found on device!");

    uint64_t size = family_count * sizeof(VkQueueFamilyProperties);
    VkQueueFamilyProperties *properties = malloc(size);
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &family_count, properties);

    for (uint32_t i = 0; i < family_count; i++) {
        if (properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queue_family.type.graphics = i;

        VkBool32 present_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, surface, &present_support);
        if (present_support)
            queue_family.type.present = i;
    }

    free(properties);
    return queue_family;
}

/**
 * @brief Create a Vulkan command pool
 * 
 * @param device Vulkan device
 * @param families Queue families
 * @return VkCommandPool New Vulkan command pool
 */
VkCommandPool vk_create_command_pool(
    VkDevice device,
    union vk_queue_family families
){
    VkCommandPoolCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    create_info.queueFamilyIndex = families.type.graphics;

    VkCommandPool command_pool;
    vkCreateCommandPool(device, &create_info, 0, &command_pool);
    return command_pool;
}

/**
 * @brief Create a Vulkan command buffer
 * 
 * @param device Vulkan device
 * @param command_pool Vulkan command pool
 * @return VkCommandBuffer New Vulkan command buffer
 */
VkCommandBuffer vk_create_command_buffer(
    VkDevice device,
    VkCommandPool command_pool
){
    // TODO: transfer command queue/buffer??
    VkCommandBufferAllocateInfo allocate_info = {0};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device, &allocate_info, &command_buffer);
    return command_buffer;
}

/**
 * @brief Get the command queues from a device using given queue families
 * 
 * @param device Vulkan device
 * @param family Queue families to query
 * @return union vk_queue Vulkan command queues
 */
union vk_queue vk_get_queues(
    VkDevice device,
    union vk_queue_family family
){
    union vk_queue queues = {0};
    for (uint32_t i = 0; i < VK_QUEUE_COUNT; i++)
        vkGetDeviceQueue(device, family.families[i], 0, &queues.queues[i]);
    
    return queues;
}
