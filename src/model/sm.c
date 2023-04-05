struct sm_header {
  uint8_t signature[4];
  uint32_t vertex_count;
  uint32_t index_count;
};

/**
 * @brief Load a .sm file from disk and create buffers for Vulkan
 * 
 * @param filename Name of the file to read and parse TODO:
 */
struct sln_model sln_load_sm(
  struct vk_state vulkan,
  char *filename
) {
  struct sln_file file = sln_read_file(filename, 1);
  struct sm_header *header = (struct sm_header *)file.data;

  struct sln_vertex *vertex_data =
    (struct sln_vertex *)((uint64_t)header + sizeof(struct sm_header));

  uint32_t *index_data = (uint32_t *)
    ((uint64_t)vertex_data + header->vertex_count * sizeof(struct sln_vertex));

  struct sln_model model = {0};

  model.vertex_buffer = vk_create_vertex_buffer(&vulkan, vertex_data,
    sizeof(struct sln_vertex) * header->vertex_count);

  model.index_buffer = vk_create_index_buffer(&vulkan, index_data,
    header->index_count);

  sln_close_file(file);

  return model;
}
