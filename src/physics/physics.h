#define PHYSICS_FLAG_EXISTS 0x1
#define PHYSICS_MAX_POINT_CUBOIDS 100

struct point_cuboid {
    uint64_t flags;
    union vector3 centre;
    union vector3 dimension;
};

struct physics_world {
    struct point_cuboid cuboid[PHYSICS_MAX_POINT_CUBOIDS];
};

#include "collision.c"
