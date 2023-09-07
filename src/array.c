#define ARRAY_USED 0x1

struct array {
    uint8_t flags;
};

/*
 * @brief Get the next available slot in an array
 * @param start Pointer to the start of the array
 * @param offset Size in bytes between each consecutive item in array. Should be
 *     sizeof(STRUCTURE)
 * @param size The size of the array (not bytes, but valid indices)
 */
struct array *next_available(struct array *start, uint64_t offset, uint64_t size) {
    for (uint64_t i = 0; i < size; i++) {
        if (!(start->flags & ARRAY_USED))
            return start;
    
        start = (struct array *)((uint64_t)start + offset);
    }
    
    return 0;
}

