/**
 * @brief Run the physics simulation
 * 
 * @param world Physics world to calculate
 */
void physics_run(
    struct physics_world *world
){
    for (uint32_t i = 0; i < SIZEOF_ARRAY(world->cuboid); i++)
        if (world->cuboid[i].flags & PHYSICS_FLAG_EXISTS)
            physics_collision_resolve(world->cuboid[i], &world->player);
}

struct physics_world physics_load_sw(
    struct sw_world world
){
    struct physics_world pworld = {0};

    for (uint32_t i = 0; i < SW_MAX_POINT_CUBOIDS
        && i < PHYSICS_MAX_POINT_CUBOIDS
        && i < world.point_cuboid_count; i++) {
        struct point_cuboid *cb = &pworld.cuboid[i];
        cb->flags = PHYSICS_FLAG_EXISTS;
        cb->centre.x = world.point_cuboids[i].centre[0];
        cb->centre.y = world.point_cuboids[i].centre[1];
        cb->centre.z = world.point_cuboids[i].centre[2];

        cb->dimension.x = world.point_cuboids[i].dimension[0];
        cb->dimension.y = world.point_cuboids[i].dimension[1];
        cb->dimension.z = world.point_cuboids[i].dimension[2];
    }

    return pworld;
}
