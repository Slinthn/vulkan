layout(set=0, binding=0) uniform world_information {
    mat4 projection;
    mat4 view;
    mat4 camera_projection;
    mat4 camera_view;
} world;

#define MAX_BONES 100

layout(set=0, binding=1) uniform animation {
    mat4 bones[MAX_BONES];
} anim;

// TODO: implement these
#define OBJECT_FLAG_TEXTURED 0x1
#define OBJECT_FLAG_COLOURED 0x2

layout(push_constant) uniform object_index {
    mat4 model;
    vec4 colour;
    uint flags;
} index;
