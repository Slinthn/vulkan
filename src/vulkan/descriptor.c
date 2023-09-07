/**
 * @brief Create a new descriptor set layout
 * 
 * @param device Vulkan device
 * @param bindings Array of descriptor set bindings
 * @param binding_count Number of elements in the bindings array
 * @return VkDescriptorSetLayout New descriptor set layout
 */
VkDescriptorSetLayout vk_create_descriptor_set_layout(
    VkDevice device,
    VkDescriptorSetLayoutBinding *bindings,
    uint32_t binding_count
){
    VkDescriptorSetLayoutCreateInfo set_create = {0};
    set_create.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_create.bindingCount = binding_count;
    set_create.pBindings = bindings;

    VkDescriptorSetLayout set_layout;
    vkCreateDescriptorSetLayout(device, &set_create, 0, &set_layout);
    return set_layout;
}

/**
 * @brief Create a new descriptor set layout. This layout should be used for
 *     struct vk_uniform_buffer0 and struct vk_uniform_buffer1
 * 
 * @param device Vulkan device
 * @return VkDescriptorSetLayout New descriptor set layout
 */
VkDescriptorSetLayout vk_create_descriptor_set_layout0(
    VkDevice device
){
    // Uniform buffer 0
    VkDescriptorSetLayoutBinding bind[2] = {0};
    bind[0].binding = 0;
    bind[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bind[0].descriptorCount = 1;
    bind[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    bind[1].binding = 1;
    bind[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bind[1].descriptorCount = 1;
    bind[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    return vk_create_descriptor_set_layout(device, bind, SIZEOF_ARRAY(bind));
}

/**
 * @brief Create a new descriptor set layout. This layout should be used for
 *     a VkSampler
 * 
 * @param device Vulkan device
 * @return VkDescriptorSetLayout New descriptor set layout
 */
VkDescriptorSetLayout vk_create_descriptor_set_layout1(
    VkDevice device
){
    // Sampler
    VkDescriptorSetLayoutBinding bind = {0};
    bind.binding = 0;
    bind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bind.descriptorCount = 1;
    bind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    return vk_create_descriptor_set_layout(device, &bind, 1);
}

/**
 * @brief Create a descriptor pool. This function is suitable to be used with
 *     vk_create_descriptor_set_layout0 and vk_create_descriptor_set_layout1
 * 
 * @param device Vulkan device
 * @param pool New descriptor pool
 */
VkDescriptorPool vk_create_descriptor_pool(
    VkDevice device
){
    VkDescriptorPoolSize size[2] = {0};
    size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    size[0].descriptorCount = 2;  // TODO: the two uniform buffers

    size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    size[1].descriptorCount = 10;  // TODO: random number

    VkDescriptorPoolCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.maxSets = VK_POOL_SIZE;
    create_info.poolSizeCount = SIZEOF_ARRAY(size);
    create_info.pPoolSizes = size;

    VkDescriptorPool pool;
    vkCreateDescriptorPool(device, &create_info, 0, &pool);
    return pool;
}

/**
 * @brief Allocate memory for descriptor sets using a descriptor pool
 * 
 * @param device Vulkan device
 * @param pool Descriptor pool to use to allocate the memory
 * @param set_layouts Array of descriptor set layouts
 * @param set_layout_count Number of elements in set_layouts
 * @return VkDescriptorSet New descriptor set
 */
VkDescriptorSet vk_allocate_descriptor_sets(
    VkDevice device,
    VkDescriptorPool pool,
    VkDescriptorSetLayout *set_layouts,
    uint32_t set_layout_count
){
    VkDescriptorSetAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = pool;
    alloc_info.descriptorSetCount = set_layout_count;
    alloc_info.pSetLayouts = set_layouts;

    VkDescriptorSet set;
    vkAllocateDescriptorSets(device, &alloc_info, &set);
    return set;
}

/**
 * @brief Update the buffers of the descriptor sets for the uniform buffers
 * 
 * @param device Vulkan device
 * @param buffer0 Buffer for uniform buffer 0
 * @param set Descriptor set
 */
void vk_update_descriptor_set0(
    VkDevice device,
    VkBuffer buffer0,
    VkBuffer buffer_anim,
    VkDescriptorSet set
){
    VkDescriptorBufferInfo buffer_info[2] = {0};
    buffer_info[0].buffer = buffer0;
    buffer_info[0].offset = 0;
    buffer_info[0].range = VK_WHOLE_SIZE;
    buffer_info[1].buffer = buffer_anim;
    buffer_info[1].offset = 0;
    buffer_info[1].range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet write[1] = {0};
    write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[0].dstSet = set;
    write[0].descriptorCount = SIZEOF_ARRAY(buffer_info);
    write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write[0].pBufferInfo = buffer_info;

    vkUpdateDescriptorSets(device, SIZEOF_ARRAY(write), write, 0, 0);
}

/**
 * @brief Update the buffers of the descriptor set for the sampler
 * 
 * @param device Vulkan device
 * @param sampler Vulkan sampler
 * @param image_view Image view
 * @param set Descriptor set
 */
void vk_update_descriptor_set1(
    VkDevice device,
    VkSampler sampler,
    VkImageView image_view,
    VkDescriptorSet set
){
    VkDescriptorImageInfo image_info = {0};
    image_info.sampler = sampler;
    image_info.imageView = image_view;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write = {0};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &image_info;

    vkUpdateDescriptorSets(device, 1, &write, 0, 0);
}
