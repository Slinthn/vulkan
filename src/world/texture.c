struct vk_texture sln_load_simg(
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

    texture = vk_create_texture(device, physical_device, command_pool,
        graphics_queue, pixel_data, bytes, header->width, header->height,
        sampler, pool, set_layout);

    sln_close_file(file);

    return texture;
}