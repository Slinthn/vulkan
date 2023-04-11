/**
 * @brief Load a .sm file from disk and create buffers for Vulkan
 * 
 * @param device Vulkan device
 * @param physical_device Vulkan physical device
 * @param filename Name of the file to read and parse
 * @return struct vk_model Information about the created model, buffers, etc.
 */
struct vk_model sln_load_sm(
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
