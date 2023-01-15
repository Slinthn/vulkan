#ifdef SLN_VULKAN

uint32_t vk_find_suitable_memory_type(VkMemoryRequirements requirements,
  VkPhysicalDeviceMemoryProperties properties, uint32_t required_flags) {

  for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
    VkMemoryPropertyFlags flags = properties.memoryTypes[i].propertyFlags;
    if ((requirements.memoryTypeBits & (1 << i))
      && (flags & required_flags) == required_flags)
      return i;
  }

  return UINT32_MAX;
}

struct vk_buffer vk_create_buffer(struct vk_buffer_info buffer_info) {
  
  struct vk_buffer buffer = {0};

  VkBufferCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  create_info.size = buffer_info.size;
  create_info.usage = buffer_info.usage;
  create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
  create_info.queueFamilyIndexCount =
    SIZEOF_ARRAY(buffer_info.queue_family.families);

  create_info.pQueueFamilyIndices = buffer_info.queue_family.families;

  vkCreateBuffer(buffer_info.device, &create_info, 0, &buffer.buffer);

  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(buffer_info.device, buffer.buffer,
    &requirements);

  VkPhysicalDeviceMemoryProperties properties;
  vkGetPhysicalDeviceMemoryProperties(buffer_info.physical_device, &properties);

  VkMemoryAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = requirements.size;
  allocate_info.memoryTypeIndex =
    vk_find_suitable_memory_type(requirements, properties, buffer_info.flags);

  vkAllocateMemory(buffer_info.device, &allocate_info, 0, &buffer.memory);

  vkBindBufferMemory(buffer_info.device, buffer.buffer, buffer.memory, 0);

  return buffer;
}

#endif  // SLN_VULKAN