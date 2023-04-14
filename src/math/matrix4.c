/**
 * @brief Set values of 4x4 matrix to its identity
 * 
 * @param m Pointer in which values are returned
 */
void mat4_identity(
    union matrix4 *m
){
    *m = (union matrix4){0};

    m->m00 = 1.0f;
    m->m11 = 1.0f;
    m->m22 = 1.0f;
    m->m33 = 1.0f;
}

/**
 * @brief Create a perspective 4x4 matrix
 * 
 * @param m Pointer in which resulting matrix is returned in
 * @param aspectratio How wide should vertices be stretched (height / width)
 * @param fov Field of view of perspective, in radians
 * @param nearz Closest rendered z coordinate
 * @param farz Furthest rendered z coordinate
 */
void mat4_perspective(
    union matrix4 *m,
    float aspectratio,
    float fov,
    float nearz,
    float farz
){
    mat4_identity(m);

    float s = 1.0f / tanf(fov / 2.0f);

    m->m00 = s;
    m->m11 = s / aspectratio;
    m->m22 = (-nearz - farz) / (nearz - farz);
    m->m23 = 2 * ((farz * nearz) / (nearz - farz));
    m->m32 = 1;
    m->m33 = 0;
}

/**
 * @brief Create an orthographic 4x4 matrix
 * 
 * @param m Pointer in which resulting matrix is returned in
 * @param left Left max coordinate
 * @param right Right max coordinate
 * @param top Top max coordinate
 * @param bottom Bottom max coordinate
 * @param nearz Closest rendered z coordinate
 * @param farz Furthest rendered z coordinate
 */
void mat4_orthographic(
    union matrix4 *m,
    float left,
    float right,
    float top,
    float bottom,
    float nearz,
    float farz
){
    mat4_identity(m);

    m->m00 = 2 / (right - left);
    m->m03 = -(right + left) / (right - left);
    m->m11 = 2 / (top - bottom);
    m->m13 = -(top + bottom) / (top - bottom);
    m->m22 = 1 / (farz - nearz);
    m->m23 = -(nearz) / (farz - nearz);
}

/**
 * @brief Create a 4x4 transformation matrix
 * 
 * @param m Pointer in which resulting matrix is returned in
 * @param transform Transform to apply to the matrix
 */
void mat4_transform(
    union matrix4 *m,
    struct transform transform
){
    mat4_identity(m);

    // Apply transformation
    m->m03 = transform.position.x;
    m->m13 = transform.position.y;
    m->m23 = transform.position.z;

    // Apply rotation
    float srx = sinf(transform.rotation.x);
    float crx = cosf(transform.rotation.x);
    float sry = sinf(transform.rotation.y);
    float cry = cosf(transform.rotation.y);
    float srz = sinf(transform.rotation.z);
    float crz = cosf(transform.rotation.z);

    m->m00 = (crz * cry);
    m->m01 = (crz * sry * srx) - (srz * crx);
    m->m02 = (crz * sry * crx) + (srz * srx);
    m->m10 = srz * cry;
    m->m11 = (srz * sry * srx) + (crz * crx);
    m->m12 = (srz * sry * crx) - (crz * srx);
    m->m20 = -sry;
    m->m21 = cry * srx;
    m->m22 = cry * crx;

    // Apply scaling
    m->m00 *= transform.scale.x;
    m->m01 *= transform.scale.y;
    m->m02 *= transform.scale.z;
    m->m10 *= transform.scale.x;
    m->m11 *= transform.scale.y;
    m->m12 *= transform.scale.z;
    m->m20 *= transform.scale.x;
    m->m21 *= transform.scale.y;
    m->m22 *= transform.scale.z;
}
