/**
 * @brief Select a suitable memory type for a buffer's requirements
 * 
 * @param requirements The requirements of the buffer, size, alignment and
 *     memory type. Returned from vkGetBufferMemoryRequirements
 * @param properties The memory allocation types of the GPU. Returned from
 *     vkGetPhysicalDeviceMemoryProperties
 * @param required_flags Required flags for the memory type to have, i.e.
 *     which stages of the rendering pipeline can view the resource
 * @return Returns the suitable memory index,
 *     or UINT32_MAX if none found
 */
uint32_t _vk_find_suitable_memory_type(VkMemoryRequirements requirements,
        VkPhysicalDeviceMemoryProperties properties, uint32_t required_flags)
{
    for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
        VkMemoryPropertyFlags flags = properties.memoryTypes[i].propertyFlags;
        uint8_t met_requirements = requirements.memoryTypeBits & (1 << i)
                && (flags & required_flags) == required_flags;
        if (met_requirements)
            return i;
    }

    return UINT32_MAX;
}

/**
 * @brief Creates a buffer
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param bytes Size of the buffer in bytes
 * @param usage Usage flag bits, i.e. the type of buffer
 * @param flags Property flag bits, how the resource can be accessed and used
 * @return struct vk_buffer Created buffer
 */
struct vk_buffer _vk_create_buffer(VkDevice device,
        VkPhysicalDevice physical_device, uint64_t bytes,
        VkBufferUsageFlagBits usage, VkMemoryPropertyFlags flags)
{
    struct vk_buffer buffer = {0};

    VkBufferCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = bytes;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(device, &create_info, 0, &buffer.buffer);

    VkMemoryRequirements requirements;
    vkGetBufferMemoryRequirements(device, buffer.buffer, &requirements);

    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);

    VkMemoryAllocateInfo allocate_info = {0};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = requirements.size;
    allocate_info.memoryTypeIndex =
            _vk_find_suitable_memory_type(requirements, properties, flags);

    vkAllocateMemory(device, &allocate_info, 0, &buffer.memory);
    vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0);
    return buffer;
}

/**
 * @brief Create a vertex buffer
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param data Data to insert into the vertex buffer
 * @param data_size Size of data in bytes
 * @return struct vk_buffer The created vertex buffer information
 */
struct vk_buffer vk_create_vertex_buffer(VkDevice device,
        VkPhysicalDevice physical_device, void *data, uint64_t data_size)
{
    struct vk_buffer buffer = _vk_create_buffer(device, physical_device,
            data_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint8_t *data_ptr;
    vkMapMemory(device, buffer.memory, 0,
            VK_WHOLE_SIZE, 0, &data_ptr);

    memcpy(data_ptr, data, data_size);
    vkUnmapMemory(device, buffer.memory);

    return buffer;
}

/**
 * @brief Create a index buffer
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param indices Index data
 * @param index_count Number of indices (elements in the array)
 * @return struct vk_buffer The created index buffer information
 */
struct vk_index_buffer vk_create_index_buffer(VkDevice device,
        VkPhysicalDevice physical_device, uint32_t *indices,
        uint32_t index_count)
{
    uint64_t data_size = sizeof(uint32_t) * index_count;

    struct vk_index_buffer buffer = {0};
    buffer.index_count = index_count;

    buffer.buffer = _vk_create_buffer(device, physical_device, data_size,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    uint8_t *data_ptr;
    vkMapMemory(device, buffer.buffer.memory, 0,
        VK_WHOLE_SIZE, 0, &data_ptr);

    memcpy(data_ptr, indices, data_size);
    vkUnmapMemory(device, buffer.buffer.memory);

    return buffer;
}

/**
 * @brief Create a uniform buffer
 *
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param data_size Size of the uniform buffer in bytes
 * @return struct vk_uniform_buffer The created uniform buffer information
 */
struct vk_uniform_buffer vk_create_uniform_buffer(VkDevice device,
        VkPhysicalDevice physical_device, uint32_t data_size)
{
    struct vk_uniform_buffer buffer = {0};
    buffer.data_size = data_size;

    buffer.buffer = _vk_create_buffer(device, physical_device, data_size,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkMapMemory(device, buffer.buffer.memory, 0, VK_WHOLE_SIZE, 0,
            &buffer.data_ptr);
    return buffer;
}

/**
 * @brief Update the data in a uniform buffer. GPU sees changes immediately
 * 
 * @param ub Uniform buffer to edit
 * @param data New data to overwrite. Note that the data size should equal
 *     the data size specified in the vk_uniform_buffer structure
 */
void vk_update_uniform_buffer(struct vk_uniform_buffer ub, void *data)
{
    memcpy(ub.data_ptr, data, ub.data_size);
}
