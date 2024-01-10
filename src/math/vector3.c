union vector3 vec3_add(union vector3 v0, union vector3 v1) {
    return (union vector3){v0.x + v1.x, v0.y + v1.y, v0.z + v1.z};
}
union vector3 vec3_sub(union vector3 v0, union vector3 v1) {
    return (union vector3){v0.x - v1.x, v0.y - v1.y, v0.z - v1.z};
}

// TODO: fast inverse square root? ;)
union vector3 vec3_norm(union vector3 v) {
    float n = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    return (union vector3){v.x / n, v.y / n, v.z / n};
}

union vector3 vec3_cross(union vector3 v0, union vector3 v1) {
    float x = v0.y * v1.z - v0.z * v1.y;
    float y = v0.z * v1.x - v0.x * v1.z;
    float z = v0.x * v1.y - v0.y * v1.x;
    return (union vector3){x, y, z};
}
