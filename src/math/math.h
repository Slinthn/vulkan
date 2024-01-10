#define PI 3.141592653589793115997963468544185161590576171875f
#define DEG_TO_RAD(deg) ((PI) / 180.0f * (deg))

#pragma warning(disable:4201)
union vector4 {
    float comp[4];
    struct {
        float x, y, z, w;
    };
};
#pragma warning(default:4201)

#pragma warning(disable:4201)
union vector3 {
    float comp[3];
    struct {
        float x, y, z;
    };
};
#pragma warning(default:4201)

#pragma warning(disable:4201)
union vector2 {
    float comp[2];
    struct {
        float x, y;
    };
};
#pragma warning(default:4201)

#pragma warning(disable:4201)
union matrix4 {
    float comp[16];
    struct {
        float m00, m10, m20, m30;
        float m01, m11, m21, m31;
        float m02, m12, m22, m32;
        float m03, m13, m23, m33;
    };
};
#pragma warning(default:4201)

struct transform {
    union vector3 position, rotation, scale;
    uint8_t unused0[4];
};

#include "matrix4.c"
#include "transform.c"
#include "vector3.c"

