void vk_create_descriptor_set_layout(
    VkDevice device,
    VkDescriptorSetLayoutBinding *bindings,
    uint32_t binding_count,
    OUT VkDescriptorSetLayout *set_layout
){
    VkDescriptorSetLayoutCreateInfo set_create = {0};
    set_create.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    set_create.bindingCount = binding_count;
    set_create.pBindings = bindings;

    vkCreateDescriptorSetLayout(device, &set_create, 0, set_layout);
}

void vk_create_descriptor_set_layout0(
    VkDevice device,
    VkDescriptorSetLayout *layout
){
    // Uniform buffer 0
    VkDescriptorSetLayoutBinding bind[2] = {0};
    bind[0].binding = 0;
    bind[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bind[0].descriptorCount = 1;
    bind[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    // Uniform buffer 1
    bind[1].binding = 1;
    bind[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bind[1].descriptorCount = 1;
    bind[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    vk_create_descriptor_set_layout(device, bind, SIZEOF_ARRAY(bind), layout);
}

void vk_create_descriptor_set_layout1(
    VkDevice device,
    VkDescriptorSetLayout *layout
){
    // Sampler
    VkDescriptorSetLayoutBinding bind = {0};
    bind.binding = 0;
    bind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bind.descriptorCount = 1;
    bind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    vk_create_descriptor_set_layout(device, &bind, 1, layout);
}

/**
 * @brief Create a descriptor pool
 * 
 * @param device Vulkan device
 * @param pool Returns the created descriptor pool
 */
void vk_create_descriptor_pool(
    VkDevice device,
    OUT VkDescriptorPool *pool
){
    VkDescriptorPoolSize size[2] = {0};
    size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    size[0].descriptorCount = 2;

    size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    size[1].descriptorCount = 10; // TODO: random number

    VkDescriptorPoolCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.maxSets = VK_POOL_SIZE;
    create_info.poolSizeCount = SIZEOF_ARRAY(size);
    create_info.pPoolSizes = size;

    vkCreateDescriptorPool(device, &create_info, 0, pool);
}

/**
 * @brief Allocate descriptor set from a set layout TODO:
 * 
 * @param device Vulkan device
 * @param pool Vulkan descriptor pool
 * @param set_layout Vulkan set layout
 * @param buffer0 Vulkan buffer (Uniform buffer 0)
 * @param buffer1 Vulkan buffer (Uniform buffer 1)
 * @param set Returns the created set
 */
void vk_allocate_descriptor_sets(
    VkDevice device,
    VkDescriptorPool pool,
    VkDescriptorSetLayout *set_layouts,
    uint32_t set_layout_count,
    OUT VkDescriptorSet *set
){
    VkDescriptorSetAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = pool;
    alloc_info.descriptorSetCount = set_layout_count;
    alloc_info.pSetLayouts = set_layouts;

    vkAllocateDescriptorSets(device, &alloc_info, set);
}

/**
 * @brief TODO:
 * 
 * @param device 
 * @param buffer0 
 * @param buffer1 
 * @param set 
 */
void vk_update_descriptor_set0(
    VkDevice device,
    VkBuffer buffer0,
    VkBuffer buffer1,
    VkDescriptorSet set
){
    VkDescriptorBufferInfo buffer_info[2] = {0};
    buffer_info[0].buffer = buffer0;
    buffer_info[0].offset = 0;
    buffer_info[0].range = VK_WHOLE_SIZE;

    buffer_info[1].buffer = buffer1;
    buffer_info[1].offset = 0;
    buffer_info[1].range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet write[2] = {0};
    write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[0].dstSet = set;
    write[0].descriptorCount = 1;
    write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write[0].pBufferInfo = &buffer_info[0];

    write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write[1].dstSet = set;
    write[1].dstBinding = 1;
    write[1].descriptorCount = 1;
    write[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write[1].pBufferInfo = &buffer_info[1];

    vkUpdateDescriptorSets(device, SIZEOF_ARRAY(write), write, 0, 0);
}

/**
 * @brief TODO:
 * 
 * @param device 
 * @param sampler 
 * @param image_view 
 * @param set 
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
