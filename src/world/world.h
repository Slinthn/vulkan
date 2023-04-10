#pragma pack(push, 1)
struct sm_header {
    uint8_t signature[4];
    uint32_t vertex_count;
    uint32_t index_count;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sw_header {
    uint8_t signature[4];
    uint32_t model_count;
    uint32_t object_count;
    uint32_t point_cuboid_count;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sw_model {
    char filename[20];
};
#pragma pack(pop)

#pragma pack(push, 1)
struct sw_object {
    uint32_t index;
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

#define SLN_WORLD_FLAG_EXISTS 0x1

struct sln_object {
    uint32_t model_index;
    uint32_t flags;
    struct transform transform;
};

struct sln_world {
    struct vk_model models[100];  // TODO: random number
    struct sln_object objects[1000];  // TODO: random number
    struct physics_world physics;
};

#include "model.c"
#include "world.c"
