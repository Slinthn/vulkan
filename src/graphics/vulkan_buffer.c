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

struct vk_buffer vk_create_buffer(struct vk_state *state, uint64_t bytes,
  VkBufferUsageFlagBits usage, VkMemoryPropertyFlags flags) {
  
  struct vk_buffer buffer = {0};

  VkBufferCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  create_info.size = bytes;
  create_info.usage = usage;
  create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
  create_info.queueFamilyIndexCount =
    SIZEOF_ARRAY(state->queue_family.families);

  create_info.pQueueFamilyIndices = state->queue_family.families;

  vkCreateBuffer(state->device, &create_info, 0, &buffer.buffer);

  VkMemoryRequirements requirements;
  vkGetBufferMemoryRequirements(state->device, buffer.buffer,
    &requirements);

  VkPhysicalDeviceMemoryProperties properties;
  vkGetPhysicalDeviceMemoryProperties(state->physical_device, &properties);

  VkMemoryAllocateInfo allocate_info = {0};
  allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocate_info.allocationSize = requirements.size;
  allocate_info.memoryTypeIndex =
    vk_find_suitable_memory_type(requirements, properties, flags);

  vkAllocateMemory(state->device, &allocate_info, 0, &buffer.memory);

  vkBindBufferMemory(state->device, buffer.buffer, buffer.memory, 0);

  return buffer;
}

struct vk_buffer vk_create_vertex_buffer(struct vk_state *state,
  void *data, uint64_t data_size) {

  struct vk_buffer buffer = vk_create_buffer(state, data_size,
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  uint8_t *data_ptr;
  vkMapMemory(state->device, buffer.memory, 0,
    VK_WHOLE_SIZE, 0, &data_ptr);

  memcpy(data_ptr, data, data_size);
  vkUnmapMemory(state->device, buffer.memory);

  return buffer;
}

#endif  // SLN_VULKAN
