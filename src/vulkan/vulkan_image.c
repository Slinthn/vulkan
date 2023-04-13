/**
 * @brief Creates an image view for the passed image
 * 
 * @param device Vulkan device
 * @param image Image to use
 * @param format Swapchain colour format
 * @param flags Flags to do with how the image should be viewed
 * @param view Returns the created image view
 */
void _vk_get_image_view(
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags flags,
    VkImageView *view
){
    VkImageViewCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format;
    create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    create_info.subresourceRange.aspectMask = flags;
    create_info.subresourceRange.levelCount = 1;
    create_info.subresourceRange.layerCount = 1;

    vkCreateImageView(device, &create_info, 0, view);
}

/**
 * @brief Create a 2D image
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param format Format of the pixels on the image
 * @param width Width of the image
 * @param height Height of the image
 * @param usage Usage flags, how the image will be used
 * @param flags Memory flags in order to find a suitable memory property
 * @return Created image information
 */
struct vk_image vk_create_image(
    VkDevice device,
    VkPhysicalDevice physical_device,
    VkFormat format,
    uint32_t width,
    uint32_t height,
    VkImageUsageFlagBits usage,
    VkMemoryPropertyFlags flags
){
    struct vk_image image = {0};

    VkImageCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.format = format;
    create_info.extent.width = width;
    create_info.extent.height = height;
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkCreateImage(device, &create_info, 0, &image.image);

    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device, image.image, &requirements);

    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);

    VkMemoryAllocateInfo allocate_info = {0};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex =
        _vk_find_suitable_memory_type(requirements, properties, flags);

    vkAllocateMemory(device, &allocate_info, 0, &image.memory);
    vkBindImageMemory(device, image.image, image.memory, 0);
    return image;
}

void _vk_transition_image(
    VkCommandBuffer command_buffer,
    VkPipelineStageFlags src_stage_flags,
    VkPipelineStageFlags dst_stage_flags,
    VkAccessFlags src_access_flags,
    VkAccessFlags dst_access_flags,
    VkImageLayout old_layout,
    VkImageLayout new_layout,
    VkImageAspectFlags aspect_mask,
    VkImage image
){
    VkImageMemoryBarrier barrier = {0};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcAccessMask = src_access_flags;
    barrier.dstAccessMask = dst_access_flags;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspect_mask;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(command_buffer, src_stage_flags, dst_stage_flags,
        0, 0, 0, 0, 0, 1, &barrier);
}

/**
 * @brief TODO:
 * 
 * @param device 
 * @param physical_device 
 * @param command_pool 
 * @param graphics_queue 
 * @param data 
 * @param bytes 
 * @param width 
 * @param height 
 */
struct vk_texture vk_create_texture(
    VkDevice device,
    VkPhysicalDevice physical_device,
    VkCommandPool command_pool,
    VkQueue graphics_queue,
    void *data,
    uint64_t bytes,
    uint32_t width,
    uint32_t height,
    VkSampler sampler,
    VkDescriptorPool pool,
    VkDescriptorSetLayout set_layout
){
    struct vk_texture tex = {0};

    struct vk_buffer staging = _vk_create_buffer(device, physical_device,
        bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint8_t *data_ptr;
    vkMapMemory(device, staging.memory, 0,
        VK_WHOLE_SIZE, 0, &data_ptr);

    memcpy(data_ptr, data, bytes);
    vkUnmapMemory(device, staging.memory);

    tex.image = vk_create_image(device, physical_device, 
        VK_FORMAT_R8G8B8A8_SRGB, width, height,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);

    _vk_transition_image(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_ASPECT_COLOR_BIT, tex.image.image);

    // Copy data
    VkBufferImageCopy region = {0};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent.width = width;
    region.imageExtent.height = height;
    region.imageExtent.depth = 1;
    vkCmdCopyBufferToImage(command_buffer, staging.buffer, tex.image.image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    _vk_transition_image(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_VERTEX_SHADER_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
        tex.image.image);

    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    vkQueueSubmit(graphics_queue, 1, &submit_info, 0);
    vkQueueWaitIdle(graphics_queue);

    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);

    _vk_allocate_descriptor_sets(device, pool, &set_layout, 1, &tex.set);

    _vk_get_image_view(device, tex.image.image, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_ASPECT_COLOR_BIT, &tex.image_view);

    _vk_update_descriptor_set1(device, sampler, tex.image_view, tex.set);

    return tex;
}

/**
 * @brief TODO:
 * 
 * @param device 
 * @param sampler 
 */
void _vk_create_sampler(
    VkDevice device,
    OUT VkSampler *sampler
){
    VkSamplerCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.magFilter = VK_FILTER_LINEAR;
    create_info.minFilter = VK_FILTER_LINEAR;
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    create_info.mipLodBias = 1;

    vkCreateSampler(device, &create_info, 0, sampler);
}
