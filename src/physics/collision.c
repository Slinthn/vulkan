/**
 * @brief TODO:
 * 
 * @param c1 
 * @param c2 
 * @return uint8_t 
 */
void physics_collision_resolve(struct point_cuboid c,
        struct point_cuboid *player)
{
    float dx = fabsf(player->centre.c.x - c.centre.c.x);
    float dy = fabsf(player->centre.c.y - c.centre.c.y);
    float dz = fabsf(player->centre.c.z - c.centre.c.z);

    int8_t sx = player->centre.c.x - c.centre.c.x > 0 ? 1 : -1;
    int8_t sy = player->centre.c.y - c.centre.c.y > 0 ? 1 : -1;
    int8_t sz = player->centre.c.z - c.centre.c.z > 0 ? 1 : -1;

    float wx = player->dimension.c.x + c.dimension.c.x;
    float wy = player->dimension.c.y + c.dimension.c.y;
    float wz = player->dimension.c.z + c.dimension.c.z;

    float tx = wx - dx;
    float ty = wy - dy;
    float tz = wz - dz;

    uint8_t collision = 0 < tx && 0 < ty && 0 < tz;
    if (collision) {
        if (tx < ty && tx < tz) {
            player->centre.c.x = c.centre.c.x + sx * wx;
        } else if (ty < tz) {
            player->centre.c.y = c.centre.c.y + sy * wy;
        } else {
            player->centre.c.z = c.centre.c.z + sz * wz;
        }
    }
}

/**
 * @brief TODO:
 * 
 * @param world 
 * @param player 
 */
void physics_run(struct physics_world world, struct point_cuboid *player)
{
    for (uint32_t i = 0; i < SIZEOF_ARRAY(world.cuboid); i++)
        if (world.cuboid[i].flags & PHYSICS_FLAG_EXISTS)
            physics_collision_resolve(world.cuboid[i], player);
}
