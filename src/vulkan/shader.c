/**
 * @brief Creates a shader module
 * 
 * @param device Vulkan device
 * @param code Shader bytecode
 * @param size Shader bytecode size in bytes
 * @param module Returns the created shader module
 */
void vk_create_shader_module(
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

    vk_create_shader_module(device, vertex_data, vertex_size, &vs.module);
    vk_create_shader_module(device, fragment_data, fragment_size, &fs.module);

    vk_create_graphics_pipeline(device, vs, fs, render_pass, pipeline_layout,
        &shader.pipeline);

    return shader;
}
