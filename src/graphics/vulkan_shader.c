#ifdef SLN_VULKAN

/**
 * @brief Create a Vulkan pipeline layout TODO:
 * 
 * @param device Vulkan device
 * @param pipeline Returns the created pipeline layout
 * @return VkResult Vulkan errors
 */
void _vk_create_pipeline_layout(VkDevice device,
  VkDescriptorSetLayout *set_layout, VkPipelineLayout *pipeline) {

  // TODO: hardcoded
  VkDescriptorSetLayoutBinding binding = {0};
  binding.binding = 0;
  binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  binding.descriptorCount = 1;
  binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutCreateInfo set_create = {0};
  set_create.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  set_create.bindingCount = 1;
  set_create.pBindings = &binding;

  vkCreateDescriptorSetLayout(device, &set_create, 0, set_layout);

  VkPipelineLayoutCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  create_info.setLayoutCount = 1;
  create_info.pSetLayouts = set_layout;

  vkCreatePipelineLayout(device, &create_info, 0, pipeline);
}

/**
 * @brief TODO:
 * 
 * @param device 
 * @param pool 
 */
void _vk_create_descriptor_pool(VkDevice device, VkDescriptorPool *pool) {

  VkDescriptorPoolSize size = {0};
  size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  size.descriptorCount = 1;  // TODO: 2 for double-buffering?

  VkDescriptorPoolCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  //create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
  create_info.maxSets = 1;  // TODO: 2 for double-buffering?
  create_info.poolSizeCount = 1;
  create_info.pPoolSizes = &size;

  vkCreateDescriptorPool(device, &create_info, 0, pool);
}

/**
 * @brief TODO:
 * 
 * @param device 
 * @param pool 
 * @param set 
 */
void _vk_allocate_descriptor_sets(VkDevice device, VkDescriptorPool pool,
  VkDescriptorSetLayout set_layout, VkBuffer buffer, VkDescriptorSet *set) {

  VkDescriptorSetAllocateInfo alloc_info = {0};
  alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  alloc_info.descriptorPool = pool;
  alloc_info.descriptorSetCount = 1;  // TODO: double buffering
  alloc_info.pSetLayouts = &set_layout;  // TODO: double buffering

  // TODO: again, double-buffering?
  vkAllocateDescriptorSets(device, &alloc_info, set);

  VkDescriptorBufferInfo buffer_info = {0};  // TODO: double buffering
  buffer_info.buffer = buffer;
  buffer_info.offset = 0;
  buffer_info.range = VK_WHOLE_SIZE;  // TODO: double buffering

  VkWriteDescriptorSet write = {0};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstSet = *set;
  write.dstBinding = 0;
  write.dstArrayElement = 0;
  write.descriptorCount = 1;  // TODO: double-buffer
  write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  write.pBufferInfo = &buffer_info;

  vkUpdateDescriptorSets(device, 1, &write, 0, 0);
}

/**
 * @brief Creates a Vulkan graphics pipeline TODO:
 * 
 * @param device Vulkan device
 * @param extent Dimensions of framebuffer
 * @param vertex_stage Vertex shader
 * @param fragment_stage Fragment shader
 * @param render_pass Vulkan render pass to attach pipeline to
 * @param pipeline Returns the created graphics pipeline
 * @return VkResult Vulkan errors
 */
void _vk_create_graphics_pipeline(struct vk_state *state,
  VkPipelineShaderStageCreateInfo vertex_stage,
  VkPipelineShaderStageCreateInfo fragment_stage,
  VkRenderPass render_pass, VkPipeline *pipeline,
  VkPipelineLayout *pipeline_layout, VkDescriptorSet *descriptor_set, struct vk_uniform_buffer *uniform_buffer) {

  *uniform_buffer = vk_create_uniform_buffer(state,
    sizeof(struct vk_uniform_buffer0));

  VkDescriptorSetLayout set_layout;
  _vk_create_pipeline_layout(state->device, &set_layout, pipeline_layout);

  VkDescriptorPool pool;
  _vk_create_descriptor_pool(state->device, &pool);

 _vk_allocate_descriptor_sets(state->device, pool,
  set_layout, uniform_buffer->buffer.buffer, descriptor_set);

  // TODO: these are hardcoded?
  VkVertexInputBindingDescription bind_desc = {0};
  bind_desc.binding = 0;
  bind_desc.stride = 32;
  bind_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription attributes[3] = {0};
  attributes[0].location = 0;
  attributes[0].binding = 0;
  attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributes[0].offset = 0;
  attributes[1].location = 1;
  attributes[1].binding = 0;
  attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
  attributes[1].offset = sizeof(float) * 3;
  attributes[2].location = 2;
  attributes[2].binding = 0;
  attributes[2].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributes[2].offset = sizeof(float) * 5;

  VkPipelineShaderStageCreateInfo stages[] = {vertex_stage, fragment_stage};

  VkPipelineVertexInputStateCreateInfo vertex_state = {0};
  vertex_state.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  
  vertex_state.vertexBindingDescriptionCount = 1;
  vertex_state.pVertexBindingDescriptions = &bind_desc;
  vertex_state.vertexAttributeDescriptionCount = SIZEOF_ARRAY(attributes);
  vertex_state.pVertexAttributeDescriptions = attributes;

  VkPipelineInputAssemblyStateCreateInfo input_assemble = {0};
  input_assemble.sType =
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

  input_assemble.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  input_assemble.primitiveRestartEnable = 0;

  VkPipelineViewportStateCreateInfo viewport_state = {0};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.scissorCount = 1;

  VkPipelineRasterizationStateCreateInfo rasterisation = {0};
  rasterisation.sType =
    VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

  rasterisation.depthClampEnable = 0;
  rasterisation.rasterizerDiscardEnable = 0;
  rasterisation.polygonMode = VK_POLYGON_MODE_FILL;
  rasterisation.lineWidth = 1;
  rasterisation.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterisation.frontFace = VK_FRONT_FACE_CLOCKWISE;
  rasterisation.depthBiasEnable = 0;

  VkPipelineMultisampleStateCreateInfo multisample = {0};
  multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisample.sampleShadingEnable = 0;

  VkPipelineColorBlendAttachmentState blend_attachments[1] = {0};
  blend_attachments[0].blendEnable = 0;
  blend_attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT
    | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
    | VK_COLOR_COMPONENT_A_BIT;

  VkPipelineColorBlendStateCreateInfo colour_blend = {0};
  colour_blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colour_blend.logicOpEnable = 0;
  colour_blend.attachmentCount = SIZEOF_ARRAY(blend_attachments);
  colour_blend.pAttachments = blend_attachments;

  VkDynamicState dynamic_states[] = {
    VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
  };

  VkPipelineDynamicStateCreateInfo dynamic_state = {0};
  dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state.dynamicStateCount = SIZEOF_ARRAY(dynamic_states);
  dynamic_state.pDynamicStates = dynamic_states;

  VkPipelineDepthStencilStateCreateInfo depth_state = {0};
  depth_state.sType =
    VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  
  depth_state.depthTestEnable = 1;
  depth_state.depthWriteEnable = 1;
  depth_state.depthCompareOp = VK_COMPARE_OP_LESS;
  // TODO: minDepthBounds, max?

  VkGraphicsPipelineCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  create_info.stageCount = 2;
  create_info.pStages = stages;
  create_info.pVertexInputState = &vertex_state;
  create_info.pInputAssemblyState = &input_assemble;
  create_info.pViewportState = &viewport_state;
  create_info.pRasterizationState = &rasterisation;
  create_info.pMultisampleState = &multisample;
  create_info.pColorBlendState = &colour_blend;
  create_info.pDynamicState = &dynamic_state;
  create_info.layout = *pipeline_layout;
  create_info.renderPass = render_pass;
  create_info.subpass = 0;
  create_info.pDepthStencilState = &depth_state;

  vkCreateGraphicsPipelines(state->device, VK_NULL_HANDLE, 1, &create_info,
    0, pipeline);
}

/**
 * @brief Creates a shader module
 * 
 * @param device Vulkan device
 * @param code Shader bytecode
 * @param size Shader bytecode size in bytes
 * @param module Returns the created shader module
 * @return VkResult Vulkan errors
 */
void _vk_create_shader_module(VkDevice device, void *code, uint64_t size,
  VkShaderModule *module) {

  VkShaderModuleCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = size;
  create_info.pCode = code;

  vkCreateShaderModule(device, &create_info, 0, module);
}

/**
 * @brief Create a vertex and fragment shader into a single graphics pipeline
 * 
 * @param state Vulkan state
 * @param vertex_data Vertex shader data
 * @param vertex_size Vertex shader size in bytes
 * @param fragment_data Fragment shader data
 * @param fragment_size Fragment shader size in bytes
 * @return struct vk_shader Created shader information
 */
struct vk_shader vk_create_shader(struct vk_state *state, void *vertex_data,
  uint64_t vertex_size, void *fragment_data, uint64_t fragment_size) {

  struct vk_shader shader = {0};

  VkPipelineShaderStageCreateInfo vertex_stage = {0};
  vertex_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertex_stage.pName = "main";

  _vk_create_shader_module(state->device, vertex_data,
    vertex_size, &vertex_stage.module);

  VkPipelineShaderStageCreateInfo fragment_stage = {0};
  fragment_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragment_stage.pName = "main";

  _vk_create_shader_module(state->device, fragment_data,
    fragment_size, &fragment_stage.module);

  _vk_create_graphics_pipeline(state, vertex_stage, fragment_stage,
    state->render_pass, &shader.pipeline,
    &shader.pipeline_layout, &shader.descriptor_set, &shader.uniform_buffer);

  return shader;
}

#endif  // SLN_VULKAN
