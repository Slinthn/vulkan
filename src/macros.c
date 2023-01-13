#define SIZEOF_ARRAY(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGN_UP(x, alignment) (((x) + ((alignment) - 1)) & ~((alignment) - 1))
