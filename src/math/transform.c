/**
 * @brief Replace values in transform structure with specified parameters
 * 
 * @param tf Transform to replace
 * @param position X, Y, Z coordinates of position
 * @param rotation X, Y, Z values of rotation (in radians)
 * @param scale X, Y, Z values of scale
 */
void tf_set(
    struct transform *tf,
    float position[3],
    float rotation[3],
    float scale[3]
){
    for (uint8_t i = 0; i < 3; i++) {
        tf->position.comp[i] = position[i];
        tf->rotation.comp[i] = rotation[i];
        tf->scale.comp[i] = scale[i];
    }
}
