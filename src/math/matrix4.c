void mat4_identity(union matrix4 *m) {

  for (uint32_t i = 0; i < 16; i++) {
    (*m).comp[i] = 0;
  }

  (*m).c.m00 = 1.0f;
  (*m).c.m11 = 1.0f;
  (*m).c.m22 = 1.0f;
  (*m).c.m33 = 1.0f;
}

void mat4_perspective(union matrix4 *m, float aspectratio, float fov,
  float nearz, float farz) {

  mat4_identity(m);

  float s = 1.0f / (tanf(fov / 2.0f));

  (*m).c.m00 = s;
  (*m).c.m11 = s / aspectratio;
  (*m).c.m22 = ((-nearz - farz) / (nearz - farz));
  (*m).c.m23 = 2 * ((farz * nearz) / (nearz - farz));
  (*m).c.m32 = 1;
  (*m).c.m33 = 0;
}

void mat4_orthographic(union matrix4 *m, float left, float right, float top,
  float bottom, float nearz, float farz) {

  mat4_identity(m);

  (*m).c.m00 = 2 / (right - left);
  (*m).c.m03 = -(right + left) / (right - left);
  (*m).c.m11 = 2 / (top - bottom);
  (*m).c.m13 = -(top + bottom) / (top - bottom);
  (*m).c.m22 = 1 / (farz - nearz);
  (*m).c.m23 = -(nearz) / (farz - nearz);
}

void mat4_transform(union matrix4 *m, struct transform transform) {

  mat4_identity(m);

  // Apply transformation
  (*m).c.m03 = transform.position.c.x;
  (*m).c.m13 = transform.position.c.y;
  (*m).c.m23 = transform.position.c.z;

  // Apply rotation
  float srx = sinf(transform.rotation.c.x);
  float crx = cosf(transform.rotation.c.x);
  float sry = sinf(transform.rotation.c.y);
  float cry = cosf(transform.rotation.c.y);
  float srz = sinf(transform.rotation.c.z);
  float crz = cosf(transform.rotation.c.z);

  (*m).c.m00 = (crz * cry);
  (*m).c.m01 = (crz * sry * srx) - (srz * crx);
  (*m).c.m02 = (crz * sry * crx) + (srz * srx);
  (*m).c.m10 = srz * cry;
  (*m).c.m11 = (srz * sry * srx) + (crz * crx);
  (*m).c.m12 = (srz * sry * crx) - (crz * srx);
  (*m).c.m20 = -sry;
  (*m).c.m21 = cry * srx;
  (*m).c.m22 = cry * crx;

  // Apply scaling
  (*m).c.m00 *= transform.scale.c.x;
  (*m).c.m01 *= transform.scale.c.y;
  (*m).c.m02 *= transform.scale.c.z;
  (*m).c.m10 *= transform.scale.c.x;
  (*m).c.m11 *= transform.scale.c.y;
  (*m).c.m12 *= transform.scale.c.z;
  (*m).c.m20 *= transform.scale.c.x;
  (*m).c.m21 *= transform.scale.c.y;
  (*m).c.m22 *= transform.scale.c.z;
}
