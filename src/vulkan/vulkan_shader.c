void _vk_create_descriptor_set_layout(
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

void _vk_create_descriptor_set_layout0(
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

    _vk_create_descriptor_set_layout(device, bind, SIZEOF_ARRAY(bind), layout);
}

void _vk_create_descriptor_set_layout1(
    VkDevice device,
    VkDescriptorSetLayout *layout
){
    // Sampler
    VkDescriptorSetLayoutBinding bind = {0};
    bind.binding = 0;
    bind.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bind.descriptorCount = 1;
    bind.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    _vk_create_descriptor_set_layout(device, &bind, 1, layout);
}

/**
 * @brief Create a Vulkan pipeline layout TODO:
 * 
 * @param device Vulkan device
 * @param set_layout Returns the created set layout
 * @param pipeline Returns the created pipeline layout
 */
void _vk_create_pipeline_layout(
    VkDevice device,
    VkDescriptorSetLayout *set_layouts,
    uint32_t set_layout_count,
    OUT VkPipelineLayout *pipeline
){
    VkPushConstantRange push_constant = {0};
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_constant.size = sizeof(struct vk_push_constant0);

    VkPipelineLayoutCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.setLayoutCount = set_layout_count;
    create_info.pSetLayouts = set_layouts;
    create_info.pushConstantRangeCount = 1;
    create_info.pPushConstantRanges = &push_constant;

    vkCreatePipelineLayout(device, &create_info, 0, pipeline);
}

/**
 * @brief Create a descriptor pool
 * 
 * @param device Vulkan device
 * @param pool Returns the created descriptor pool
 */
void _vk_create_descriptor_pool(
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
void _vk_allocate_descriptor_sets(
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
void _vk_update_descriptor_set0(
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
void _vk_update_descriptor_set1(
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

/**
 * @brief Creates a Vulkan graphics pipeline TODO:
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param vertex_stage Vertex shader
 * @param fragment_stage Fragment shader
 * @param render_pass Vulkan render pass to attach pipeline to
 * @param pipeline Returns the created graphics pipeline
 * @param pipeline_layout Returns the created pipeline layout
 * @param descriptor_set Returns the created descriptor set
 * @param uniform_buffer0 Returns the created uniform buffer 0
 * @param uniform_buffer1 Returns the created uniform buffer 1
 */
void _vk_create_graphics_pipeline(
    VkDevice device,
    VkPipelineShaderStageCreateInfo vertex_stage,
    VkPipelineShaderStageCreateInfo fragment_stage,
    VkRenderPass render_pass,
    VkPipelineLayout pipeline_layout,
    OUT VkPipeline *pipeline
){
    VkVertexInputBindingDescription bind_desc = {0};
    bind_desc.binding = 0;
    bind_desc.stride = sizeof(struct vk_vertex);
    bind_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributes[3] = {0};
    attributes[0].location = 0;
    attributes[0].binding = 0;
    attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[0].offset = 0;

    attributes[1].location = 1;
    attributes[1].binding = 0;
    attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[1].offset = sizeof(float) * 3;

    attributes[2].location = 2;
    attributes[2].binding = 0;
    attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[2].offset = sizeof(float) * 6;

    VkPipelineShaderStageCreateInfo stages[] = {
        vertex_stage,
        fragment_stage
    };

    VkPipelineVertexInputStateCreateInfo vs = {0};
    vs.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vs.vertexBindingDescriptionCount = 1;
    vs.pVertexBindingDescriptions = &bind_desc;
    vs.vertexAttributeDescriptionCount = SIZEOF_ARRAY(attributes);
    vs.pVertexAttributeDescriptions = attributes;

    VkPipelineInputAssemblyStateCreateInfo ia = {0};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    ia.primitiveRestartEnable = 0;

    VkPipelineViewportStateCreateInfo view = {0};
    view.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    view.viewportCount = 1;
    view.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rast = {0};
    rast.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rast.depthClampEnable = 0;
    rast.rasterizerDiscardEnable = 0;
    rast.polygonMode = VK_POLYGON_MODE_FILL;
    rast.lineWidth = 1;
    rast.cullMode = VK_CULL_MODE_BACK_BIT;
    rast.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rast.depthBiasEnable = 0;

    VkPipelineMultisampleStateCreateInfo ms = {0};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms.sampleShadingEnable = 0;

    VkPipelineColorBlendAttachmentState blend_attachment = {0};
    blend_attachment.blendEnable = 0;
    blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
        | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
        | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo cb = {0};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.logicOpEnable = 0;
    cb.attachmentCount = 1;
    cb.pAttachments = &blend_attachment;

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state = {0};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = SIZEOF_ARRAY(dynamic_states);
    dynamic_state.pDynamicStates = dynamic_states;

    VkPipelineDepthStencilStateCreateInfo ds = {0};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = 1;
    ds.depthWriteEnable = 1;
    ds.depthCompareOp = VK_COMPARE_OP_LESS;

    VkGraphicsPipelineCreateInfo info = {0};
    info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.stageCount = 2;
    info.pStages = stages;
    info.pVertexInputState = &vs;
    info.pInputAssemblyState = &ia;
    info.pViewportState = &view;
    info.pRasterizationState = &rast;
    info.pMultisampleState = &ms;
    info.pDepthStencilState = &ds;
    info.pColorBlendState = &cb;
    info.pDynamicState = &dynamic_state;
    info.layout = pipeline_layout;
    info.renderPass = render_pass;
    info.subpass = 0;

    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, 0, pipeline);
}

/**
 * @brief Creates a shader module
 * 
 * @param device Vulkan device
 * @param code Shader bytecode
 * @param size Shader bytecode size in bytes
 * @param module Returns the created shader module
 */
void _vk_create_shader_module(
    VkDevice device,
    void *code,
    uint64_t bytes,
    OUT VkShaderModule *module
){
    VkShaderModuleCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = bytes;
    create_info.pCode = code;

    vkCreateShaderModule(device, &create_info, 0, module);
}

/**
 * @brief Create a vertex and fragment shader into a single graphics pipeline
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param render_pass Vulkan render pass
 * @param vertex_data Vertex shader data
 * @param vertex_size Vertex shader size in bytes
 * @param fragment_data Fragment shader data
 * @param fragment_size Fragment shader size in bytes
 * @return struct vk_shader Created shader information
 */
struct vk_shader vk_create_shader(
    VkDevice device,
    VkRenderPass render_pass,
    void *vertex_data,
    uint64_t vertex_size,
    void *fragment_data,
    uint64_t fragment_size,
    VkPipelineLayout pipeline_layout
){
    struct vk_shader shader = {0};

    VkPipelineShaderStageCreateInfo vs = {0};
    vs.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vs.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vs.pName = "main";

    VkPipelineShaderStageCreateInfo fs = {0};
    fs.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fs.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fs.pName = "main";

    _vk_create_shader_module(device, vertex_data, vertex_size, &vs.module);
    _vk_create_shader_module(device, fragment_data, fragment_size, &fs.module);

    _vk_create_graphics_pipeline(device, vs, fs, render_pass, pipeline_layout,
        &shader.pipeline);

    return shader;
}
