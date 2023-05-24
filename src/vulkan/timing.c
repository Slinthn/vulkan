/**
 * @brief Create a Vulkan semaphore
 * 
 * @param device Vulkan device
 * @return VkSemaphore New Vulkan semaphore
 */
VkSemaphore vk_create_semaphore(
    VkDevice device
){
    VkSemaphoreCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkSemaphore semaphore;
    vkCreateSemaphore(device, &create_info, 0, &semaphore);
    return semaphore;
}

/**
 * @brief Create a Vulkan fence
 * 
 * @param device Vulkan device
 * @return VkFence New Vulkan fence
 */
VkFence vk_create_fence(
    VkDevice device
){
    VkFenceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence;
    vkCreateFence(device, &create_info, 0, &fence);
    return fence;
}
