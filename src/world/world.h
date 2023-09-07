#pragma pack(push, 1)
struct sm_header {
    uint8_t signature[4];
    uint32_t vertex_count;
    uint32_t index_count;
    union vector4 colour;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sw_header {
    uint8_t signature[4];
    uint32_t model_count;
    uint32_t texture_count;
    uint32_t object_count;
    uint32_t point_cuboid_count;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct simg_header {
    uint8_t signature[4];
    uint32_t width;
    uint32_t height;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sa_header {
    uint8_t signature[4];
    uint32_t bone_count;
    uint32_t keyframe_count;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sw_model {
    char filename[20];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sw_texture {
    char filename[20];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sw_object {
    uint32_t model_index;
    uint32_t texture_index;
    float position[3];
    float rotation[3];
    float scale[3];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sw_point_cuboid {
    float centre[3];
    float dimension[3];
};
#pragma pack(pop)

#define SW_FLAG_EXISTS 0x1

struct sln_object {
    uint32_t model_index;
    uint32_t texture_index;
    uint64_t flags;
    struct transform transform;
};

#define SW_MAX_MODELS 100
#define SW_MAX_TEXTURES SW_MAX_MODELS
#define SW_MAX_OBJECTS SW_MAX_MODELS
#define SW_MAX_POINT_CUBOIDS SW_MAX_MODELS

struct sw_world {
    uint64_t model_count;
    struct sw_model models[SW_MAX_MODELS];
    uint64_t texture_count;
    struct sw_texture textures[SW_MAX_TEXTURES];
    uint64_t object_count;
    struct sw_object objects[SW_MAX_OBJECTS];
    uint64_t point_cuboid_count;
    struct sw_point_cuboid point_cuboids[SW_MAX_POINT_CUBOIDS];
};

#include "world.c"
