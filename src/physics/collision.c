/**
 * @brief Detect a collision between the player and an object, and also resolve
 *     this collision
 * 
 * @param c1 Cuboid to test
 * @param c2 Player to test
 */
void physics_collision_resolve(
    struct point_cuboid c,
    struct point_cuboid *player
){
    float dx = fabsf(player->centre.x - c.centre.x);
    float dy = fabsf(player->centre.y - c.centre.y);
    float dz = fabsf(player->centre.z - c.centre.z);

    int8_t sx = player->centre.x - c.centre.x > 0 ? 1 : -1;
    int8_t sy = player->centre.y - c.centre.y > 0 ? 1 : -1;
    int8_t sz = player->centre.z - c.centre.z > 0 ? 1 : -1;

    float wx = player->dimension.x + c.dimension.x;
    float wy = player->dimension.y + c.dimension.y;
    float wz = player->dimension.z + c.dimension.z;

    float tx = wx - dx;
    float ty = wy - dy;
    float tz = wz - dz;

    uint8_t collision = 0 < tx && 0 < ty && 0 < tz;
    if (!collision)
        return;

    if (tx < ty && tx < tz)
        player->centre.x = c.centre.x + sx * wx;
    else if (ty < tz)
        player->centre.y = c.centre.y + sy * wy;
    else
        player->centre.z = c.centre.z + sz * wz;
}
