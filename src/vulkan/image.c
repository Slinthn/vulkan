/**
 * @brief Creates an image view for the passed image
 * 
 * @param device Vulkan device
 * @param image Image to use
 * @param format Swapchain colour format
 * @param flags Flags to do with how the image should be viewed
 * @return VkImageView Created image view
 */
VkImageView vk_get_image_view(
    VkDevice device,
    VkImage image,
    VkFormat format,
    VkImageAspectFlags flags
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

    VkImageView view;
    vkCreateImageView(device, &create_info, 0, &view);
    return view;
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
 * @return struct vk_image Created image information
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
        vk_find_suitable_memory_type(requirements, properties, flags);

    vkAllocateMemory(device, &allocate_info, 0, &image.memory);
    vkBindImageMemory(device, image.image, image.memory, 0);
    return image;
}

/**
 * @brief Transition an image resource barrier. Must be called in between a
 *     vkBeginCommandBuffer and vEndCommandBuffer
 * 
 * @param command_buffer Command buffer to execute command on
 * @param src_stage_flags Old memory access types that can read/write to image
 * @param dst_stage_flags New memory access types that can read/write to image
 * @param src_access_flags Old stages which can access this resource
 * @param dst_access_flags New stages which can access this resource
 * @param old_layout Old accessing method
 * @param new_layout New accessing method
 * @param aspect_mask Which aspect of the image is accessed (colour, depth, etc)
 * @param image Image to act upon
 */
void vk_transition_image(
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
 * @brief Create a texture
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param command_pool Vulkan command pool
 * @param graphics_queue Vulkan graphics queue
 * @param data Texture pixel data
 * @param bytes Size of data in bytes
 * @param width Texture width
 * @param height Texture Height
 * @param sampler Vulkan sampler to use for this texture
 * @param pool Vulkan descriptor pool
 * @param set_layout Vulkan set layout for a texture
 * @return struct vk_texture Created texture information
 */
struct vk_texture vk_create_texture(
    VkDevice device,
    VkPhysicalDevice physical_device,
    VkCommandPool command_pool,  // TODO: i hate this, a command pool and queue?
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

    struct vk_buffer staging = vk_create_buffer(device, physical_device,
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

    vk_transition_image(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
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

    vk_transition_image(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
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

    vk_allocate_descriptor_sets(device, pool, &set_layout, 1, &tex.set);

    tex.image_view = vk_get_image_view(device, tex.image.image,
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    vk_update_descriptor_set1(device, sampler, tex.image_view, tex.set);

    return tex;
}

/**
 * @brief Create a sampler for a texture TODO:
 * 
 * @param device Vulkan device
 * @return VkSampler Vulkan sampler 
 */
VkSampler vk_create_nearest_sampler(
    VkDevice device
){
    VkSamplerCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.magFilter = VK_FILTER_NEAREST;  // TODO: parameter?
    create_info.minFilter = VK_FILTER_NEAREST;
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    create_info.mipLodBias = 1;

    VkSampler sampler;
    vkCreateSampler(device, &create_info, 0, &sampler);
    return sampler;
}

/**
 * @brief Create a sampler for a texture TODO:
 * 
 * @param device Vulkan device
 * @return VkSampler Vulkan sampler 
 */
VkSampler vk_create_linear_sampler(
    VkDevice device
){
    VkSamplerCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.magFilter = VK_FILTER_LINEAR;  // TODO: parameter?
    create_info.minFilter = VK_FILTER_LINEAR;
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    create_info.mipLodBias = 1;

    VkSampler sampler;
    vkCreateSampler(device, &create_info, 0, &sampler);
    return sampler;
}

/**
 * @brief Create a depth buffer TODO:
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @return struct vk_image 
 */
struct vk_image vk_create_depth_buffer(
    VkDevice device,
    VkExtent2D extent,
    VkPhysicalDevice pd
){
    return vk_create_image(device, pd, VK_FORMAT_D32_SFLOAT,
        extent.width, extent.height,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

/**
 * @brief Create a depth buffer
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @return struct vk_image 
 */
struct vk_image vk_create_shadow_depth_buffer(
    VkDevice device,
    VkPhysicalDevice pd
){
    return vk_create_image(device, pd, VK_FORMAT_D32_SFLOAT,
        VK_SHADOW_WIDTH, VK_SHADOW_HEIGHT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}
