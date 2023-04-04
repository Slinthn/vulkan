#ifdef SLN_VULKAN

/**
 * @brief Create a Vulkan pipeline layout
 * 
 * @param device Vulkan device
 * @param pipeline Returns the created pipeline layout
 * @return VkResult Vulkan errors
 */
void vk_create_pipeline_layout(VkDevice device,
  VkPipelineLayout *pipeline) {

  VkPipelineLayoutCreateInfo create_info = {0};
  create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  vkCreatePipelineLayout(device, &create_info, 0, pipeline);
}

/**
 * @brief Creates a Vulkan graphics pipeline
 * 
 * @param device Vulkan device
 * @param extent Dimensions of framebuffer
 * @param vertex_stage Vertex shader
 * @param fragment_stage Fragment shader
 * @param render_pass Vulkan render pass to attach pipeline to
 * @param bind_desc Vulkan bind description, i.e. stride and vertex format
 * @param attributes Array of vertex attributes, i.e. position, texture, normal
 * @param attribute_count Number of items in attributes array
 * @param pipeline Returns the created graphics pipeline
 * @return VkResult Vulkan errors
 */
void _vk_create_graphics_pipeline(VkDevice device,
  VkPipelineShaderStageCreateInfo vertex_stage,
  VkPipelineShaderStageCreateInfo fragment_stage,
  VkRenderPass render_pass,
  VkVertexInputBindingDescription bind_desc,
  VkVertexInputAttributeDescription *attributes,
  uint32_t attribute_count,
  VkPipeline *pipeline) {

  VkPipelineLayout pipeline_layout;
  vk_create_pipeline_layout(device, &pipeline_layout);

  VkPipelineShaderStageCreateInfo stages[] =
    {vertex_stage, fragment_stage};

  VkPipelineVertexInputStateCreateInfo vertex_state = {0};
  vertex_state.sType =
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  
  vertex_state.vertexBindingDescriptionCount = 1;
  vertex_state.pVertexBindingDescriptions = &bind_desc;
  vertex_state.vertexAttributeDescriptionCount = attribute_count;
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
  create_info.layout = pipeline_layout;
  create_info.renderPass = render_pass;
  create_info.subpass = 0;

  vkCreateGraphicsPipelines(device, VK_NULL_HANDLE,
    1, &create_info, 0, pipeline);
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

  _vk_create_graphics_pipeline(state->device, vertex_stage, fragment_stage,
    state->render_pass, bind_desc, attributes, SIZEOF_ARRAY(attributes),
    &shader.pipeline);

  return shader;
}

#endif  // SLN_VULKAN
