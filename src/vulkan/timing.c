/**
 * @brief Create a Vulkan semaphore
 * 
 * @param device Vulkan device
 * @param semaphore Returns the created semaphore
 */
void vk_create_semaphore(
    VkDevice device,
    OUT VkSemaphore *semaphore
){
    VkSemaphoreCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateSemaphore(device, &create_info, 0, semaphore);
}

/**
 * @brief Create a Vulkan fence
 * 
 * @param device Vulkan device
 * @param fence Returns the created fence
 */
void vk_create_fence(
    VkDevice device,
    VkFence *fence
){
    VkFenceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    vkCreateFence(device, &create_info, 0, fence);
}
