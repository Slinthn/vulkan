#define PHYSICS_FLAG_EXISTS 0x1

struct point_cuboid {
    uint64_t flags;
    union vector3 centre;
    union vector3 dimension;
};

struct physics_world {
    struct point_cuboid cuboid[100];  // TODO: random number
};

#include "collision.c"
