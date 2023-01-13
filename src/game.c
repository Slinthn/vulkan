/**
 * @brief Main game code 
 *  
 */

struct sln_file {
  void *data;
  uint64_t size;
  uint64_t allocated_size;
};

struct sln_file sln_read_file(char *filename, uint64_t alignment) {
  struct sln_file ret = {0}; 
    
  FILE *file = fopen(filename, "rb");

  fseek(file, 0, SEEK_END);
  ret.size = ftell(file);
  fseek(file, 0, SEEK_SET);

  ret.allocated_size = ALIGN_UP(ret.size, alignment);

  ret.data = calloc(1, ret.allocated_size);
  fread(ret.data, ret.size, 1, file);

  fclose(file);

  return ret;
}

void sln_init(struct sln_app app) {

  struct vk_state vulkan = vk_init(app);

  struct sln_file vertex_file = sln_read_file("shader-v.spv", 4);
  struct sln_file fragment_file = sln_read_file("shader-f.spv", 4);


  VkPipelineShaderStageCreateInfo vertex_stage = {0};
  vertex_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertex_stage.pName = "main";
  vk_create_shader_module(vulkan.device, vertex_file.data,
    vertex_file.allocated_size, &vertex_stage.module);

  VkPipelineShaderStageCreateInfo fragment_stage = {0};
  fragment_stage.sType =
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

  fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragment_stage.pName = "main";
  vk_create_shader_module(vulkan.device, fragment_file.data,
    fragment_file.allocated_size, &fragment_stage.module);


  VkPipeline pipeline;
  vk_create_graphics_pipeline(vulkan.device, vertex_stage, fragment_stage,
    vulkan.render_pass, &pipeline);
}
