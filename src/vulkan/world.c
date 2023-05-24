/**
 * @brief Load a .sm file from disk and create buffers for Vulkan
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param filename Name of the file to read and parse
 * @return struct vk_model Information about the created model, buffers, etc.
 */
struct vk_model graphics_load_sm(
    VkDevice device,
    VkPhysicalDevice physical_device,
    char *filename
){
    struct sln_file file = sln_read_file(filename, 1);
    struct sm_header *header = (struct sm_header *)file.data;

    struct vk_vertex *vertex_data =
        (struct vk_vertex *)((uint64_t)header + sizeof(struct sm_header));

    uint64_t index_offset = (uint64_t)vertex_data
        + header->vertex_count * sizeof(struct vk_vertex);
    uint32_t *index_data = (uint32_t *)index_offset;

    struct vk_model model = {0};

    model.vertex_buffer = vk_create_vertex_buffer(device, physical_device,
        vertex_data, sizeof(struct vk_vertex) * header->vertex_count);

    model.index_buffer = vk_create_index_buffer(device, physical_device,
        index_data, header->index_count);

    sln_close_file(file);

    return model;
}

/**
 * @brief Load .simg from disk and create a Vulkan image
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param command_pool Vulkan command pool
 * @param graphics_queue Vulkan graphics queue
 * @param sampler Sampler to use with the image
 * @param pool Vulkan descriptor pool to allocate new set
 * @param set_layout Vulkan set layout to allocate
 * @param filename SIMG file name
 * @return struct vk_texture 
 */
struct vk_texture graphics_load_simg(
    VkDevice device,
    VkPhysicalDevice physical_device,
    VkCommandPool command_pool,
    VkQueue graphics_queue,
    VkSampler sampler,
    VkDescriptorPool pool,
    VkDescriptorSetLayout set_layout,
    char *filename
){
    struct sln_file file = sln_read_file(filename, 1);
    struct simg_header *header = (struct simg_header *)file.data;

    uint32_t *pixel_data = (uint32_t *)
        ((uint64_t)header + sizeof(struct simg_header));

    struct vk_texture texture = {0};

    uint64_t bytes = 4 * header->width * header->height;

    // TODO: could i just create a command queue here and delete it after?
    // instead of doing it with each function call
    texture = vk_create_texture(device, physical_device, command_pool,
        graphics_queue, pixel_data, bytes, header->width, header->height,
        sampler, pool, set_layout);

    sln_close_file(file);

    return texture;
}

/**
 * @brief Load .sw file from disk
 * 
 * @param state Graphics state
 * @param world Graphics world
 * @return struct graphics_world New graphics world
 */
struct graphics_world graphics_load_sw(
    struct graphics_state state,
    struct sw_world world
){
    struct graphics_world gworld = {0};

    for (uint32_t i = 0; i < VK_MAX_MODELS && i < SW_MAX_MODELS
        && i < world.model_count; i++)
        gworld.models[i] = graphics_load_sm(state.device,
            state.physical_device, world.models[i].filename);

    for (uint32_t i = 0; i < VK_MAX_TEXTURES && i < SW_MAX_TEXTURES
        && i < world.texture_count; i++)
        gworld.textures[i] = graphics_load_simg(state.device,
            state.physical_device, state.command_pool,
            state.queue.type.graphics, state.sampler, state.pool,
            state.set_layout[2], world.textures[i].filename);

    for (uint32_t i = 0; i < VK_MAX_OBJECTS && i < SW_MAX_OBJECTS
        && i < world.object_count; i++) {
        gworld.objects[i].model = &gworld.models[world.objects[i].model_index];
        gworld.objects[i].texture =
            &gworld.textures[world.objects[i].texture_index];
        gworld.objects[i].flags = VK_FLAG_EXISTS;
        tf_set(&gworld.objects[i].transform, world.objects[i].position,
            world.objects[i].rotation, world.objects[i].scale);
    }

    return gworld;
}
