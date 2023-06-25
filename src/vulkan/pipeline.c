/**
 * @brief Create a Vulkan pipeline layout
 * 
 * @param device Vulkan device
 * @param set_layouts Array of set layouts
 * @param set_layout_count Number of elements in set_layouts
 * @return VkPipelineLayout New pipeline layout
 */
VkPipelineLayout vk_create_pipeline_layout(
    VkDevice device,
    VkDescriptorSetLayout *set_layouts,
    uint32_t set_layout_count
){
    VkPushConstantRange push_constant[1] = {0};
    push_constant[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant[0].size = sizeof(struct vk_push_constant0);

    VkPipelineLayoutCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.setLayoutCount = set_layout_count;
    create_info.pSetLayouts = set_layouts;
    create_info.pushConstantRangeCount = SIZEOF_ARRAY(push_constant);
    create_info.pPushConstantRanges = push_constant;

    VkPipelineLayout pipeline;
    vkCreatePipelineLayout(device, &create_info, 0, &pipeline);
    return pipeline;
}

/**
 * @brief Create a Vulkan graphics pipeline
 * 
 * @param device Vulkan device
 * @param vertex_stage Vertex shader
 * @param fragment_stage Fragment shader
 * @param render_pass Vulkan render pass to attach pipeline to
 * @param pipeline_layout Vulkan pipeline layout to use
 * @param culling Cull type
 */
VkPipeline vk_create_graphics_pipeline(
    VkDevice device,
    VkPipelineShaderStageCreateInfo vertex_stage,
    VkPipelineShaderStageCreateInfo fragment_stage,
    VkRenderPass render_pass,
    VkPipelineLayout pipeline_layout,
    VkCullModeFlags culling
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
    rast.cullMode = culling;
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
    
    VkPipeline pipeline;
    vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, 0, &pipeline);
    return pipeline;
}
