/**
 * @brief Load a .sm file from disk and create buffers for Vulkan
 * 
 * @param filename Name of the file to read and parse
 * @return struct sw_world Information about the created world
 */
struct sw_world sln_load_sw(
    char *filename
){
    struct sw_world world = {0};
    struct sln_file file = sln_read_file(filename, 1);
    struct sw_header *header = (struct sw_header *)file.data;

    struct sw_model *model_ptr = (struct sw_model *)((uint64_t)header +
        sizeof(struct sw_header));

    uint32_t i;
    for (i = 0; i < header->model_count; i++, model_ptr++)
        world.models[i] = *model_ptr;

    world.model_count = i;

    struct sw_texture *texture_ptr = (struct sw_texture *)model_ptr;
    for (i = 0; i < header->texture_count; i++, texture_ptr++)
        world.textures[i] = *texture_ptr;

    world.texture_count = i;

    struct sw_object *object_ptr = (struct sw_object *)texture_ptr;
    for (i = 0; i < header->object_count; i++, object_ptr++)
        world.objects[i] = *object_ptr;

    world.object_count = i;

    struct sw_point_cuboid *cb_ptr = (struct sw_point_cuboid *)object_ptr;
    for (i = 0; i < header->point_cuboid_count; i++, cb_ptr++)
        world.point_cuboids[i] = *cb_ptr;

    world.point_cuboid_count = i;

    sln_close_file(file);
    return world;
}
