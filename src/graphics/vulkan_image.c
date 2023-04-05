#ifdef SLN_VULKAN

/**
 * @brief TODO:
 * 
 * @param device 
 */
struct vk_image vk_create_image(struct vk_state *state, VkFormat format, uint32_t width,
  uint32_t height, VkImageUsageFlagBits usage, VkMemoryPropertyFlags flags) {

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

  vkCreateImage(state->device, &create_info, 0, &image.image);

  VkMemoryRequirements requirements;
  vkGetImageMemoryRequirements(state->device, image.image, &requirements);

  VkPhysicalDeviceMemoryProperties properties;
  vkGetPhysicalDeviceMemoryProperties(state->physical_device, &properties);

  VkMemoryAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = requirements.size;
  allocate_info.memoryTypeIndex =
    _vk_find_suitable_memory_type(requirements, properties, flags);

  vkAllocateMemory(state->device, &allocate_info, 0, &image.memory);

  vkBindImageMemory(state->device, image.image, image.memory, 0);

  return image;
}

struct vk_image vk_create_depth_buffer(struct vk_state *state) {

  return vk_create_image(state, VK_FORMAT_D32_SFLOAT, SLN_WINDOW_WIDTH,
    SLN_WINDOW_HEIGHT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
}

#endif  // SLN_VULKAN
