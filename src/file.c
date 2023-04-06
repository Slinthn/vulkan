struct sln_file {
    void *data;
    uint64_t size;
    uint64_t allocated_size;
};

/**
 * @brief Read the contents of a file to a buffer
 * 
 * @param filename The name of the file to read
 * @param alignment Whether the allocated memory size needs to be aligned
 * @return struct sln_file The created and read file
 */
struct sln_file sln_read_file(char *filename, uint64_t alignment)
{
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

/**
 * @brief Delete the memory of a file
 * 
 * @param file The file of which the memory is to be freed
 */
void sln_close_file(struct sln_file file)
{
    free(file.data);
}
