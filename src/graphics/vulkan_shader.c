#ifdef SLN_VULKAN

struct vk_shader vk_create_shader(struct vk_shader_info shader_info) {

  struct vk_shader shader = {0};

  VkPipelineShaderStageCreateInfo vertex_stage = {0};
  vertex_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertex_stage.pName = "main";

  vk_create_shader_module(shader_info.device, shader_info.vertex_data,
    shader_info.vertex_data_size, &vertex_stage.module);

  VkPipelineShaderStageCreateInfo fragment_stage = {0};
  fragment_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragment_stage.pName = "main";

  vk_create_shader_module(shader_info.device, shader_info.fragment_data,
    shader_info.fragment_data_size, &fragment_stage.module);
  
  vk_create_graphics_pipeline(shader_info.device, vertex_stage, fragment_stage,
    shader_info.render_pass, &shader.pipeline);

  return shader;
}

#endif  // SLN_VULKAN