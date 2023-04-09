/**
 * @brief Load a .sm file from disk and create buffers for Vulkan
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param filename Name of the file to read and parse
 * @return struct sln_world Information about the created world
 */
struct sln_world sln_load_sw(VkDevice device, VkPhysicalDevice physical_device,
        char *filename)
{
    struct sln_world world = {0};
    struct sln_file file = sln_read_file(filename, 1);
    struct sw_header *header = (struct sw_header *)file.data;

    struct sw_model *model_ptr = (struct sw_model *)((uint64_t)header +
            sizeof(struct sw_header));

    for (uint32_t i = 0; i < header->model_count; i++, model_ptr++)
        world.models[i] = sln_load_sm(device, physical_device,
                model_ptr->filename);

    struct sw_object *object_ptr = (struct sw_object *)model_ptr;

    for (uint32_t i = 0; i < header->object_count; i++, object_ptr++) {
        struct sln_object *object = &world.objects[i];
        object->model_index = object_ptr->index;
        object->flags = SLN_WORLD_FLAG_EXISTS;
        tf_set(&object->transform, object_ptr->position, object_ptr->rotation,
                object_ptr->scale);
    }

    sln_close_file(file);
    return world;
}
