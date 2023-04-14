/**
 * @brief Load a .sm file from disk and create buffers for Vulkan
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param filename Name of the file to read and parse
 * @return struct sln_world Information about the created world
 */
struct sln_world sln_load_sw(
    VkDevice device,
    VkPhysicalDevice physical_device,
    VkCommandPool command_pool,
    VkQueue graphics_queue,
    VkSampler sampler,
    VkDescriptorPool pool,
    VkDescriptorSetLayout set_layout,
    char *filename
){
    struct sln_world world = {0};
    struct sln_file file = sln_read_file(filename, 1);
    struct sw_header *header = (struct sw_header *)file.data;

    // Models
    struct sw_model *model_ptr = (struct sw_model *)((uint64_t)header +
        sizeof(struct sw_header));

    for (uint32_t i = 0; i < header->model_count; i++, model_ptr++)
        world.models[i] = sln_load_sm(device, physical_device,
            model_ptr->filename);

    // Textures
    struct sw_model *texture_ptr = (struct sw_model *)model_ptr;

    for (uint32_t i = 0; i < header->texture_count; i++, texture_ptr++)
        world.textures[i] = sln_load_simg(device, physical_device, command_pool,
            graphics_queue, sampler, pool, set_layout, texture_ptr->filename);

    // Objects
    struct sw_object *object_ptr = (struct sw_object *)texture_ptr;

    for (uint32_t i = 0; i < header->object_count; i++, object_ptr++) {
        struct sln_object *object = &world.objects[i];
        object->model_index = object_ptr->model_index;
        object->texture_index = object_ptr->texture_index;
        object->flags = SLN_WORLD_FLAG_EXISTS;
        tf_set(&object->transform, object_ptr->position, object_ptr->rotation,
            object_ptr->scale);
    }

    // Point Cuboids
    struct sw_point_cuboid *cb_ptr = (struct sw_point_cuboid *)object_ptr;

    for (uint32_t i = 0; i < header->point_cuboid_count; i++, cb_ptr++) {
        struct point_cuboid *cb = &world.physics.cuboid[i];
        cb->flags = PHYSICS_FLAG_EXISTS;
        cb->centre.x = cb_ptr->centre[0];
        cb->centre.y = cb_ptr->centre[1];
        cb->centre.z = cb_ptr->centre[2];

        cb->dimension.x = cb_ptr->dimension[0];
        cb->dimension.y = cb_ptr->dimension[1];
        cb->dimension.z = cb_ptr->dimension[2];
    }

    sln_close_file(file);
    return world;
}
