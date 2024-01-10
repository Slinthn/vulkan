#version 450

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in ivec4 bone_ids;
layout(location=3) in vec4 bone_weights;

layout(location=0) out vec2 tex;
layout(location=1) out vec4 shadow_tex;
layout(location=2) out vec4 o_position;
layout(location=3) out vec3 o_normal;
layout(location=4) out vec2 o_tex;


#include "common.glsl"

#define TMP_MUL 1

void main(
    void
){
    vec4 pos = vec4(position.x, -position.y, position.z, 1);
    vec4 mul_pos = vec4(pos.x, pos.y, pos.z, 1);
    vec4 total_pos = pos;
    mat4 total_mat = mat4(0);

    if (bone_weights[0] + bone_weights[1] + bone_weights[2] + bone_weights[3] == 0) {
        total_mat = mat4(1);
    } else {

        for (int i = 0; i < 4; i++) {
            vec4 add = (TMP_MUL * bone_weights[i]) * (anim.bones[bone_ids[i]] * mul_pos);
            total_mat += bone_weights[i] * (anim.bones[bone_ids[i]]);
            total_pos += add;
        }
    }

    gl_Position = world.projection
        * inverse(world.view)
        * index.model * total_mat * pos;

    shadow_tex = world.camera_projection
        * inverse(world.camera_view)
        * index.model
        * vec4(position.x, -position.y, position.z, 1);

    o_position = vec4(position.xyz, 1);
    o_normal = normal;
    o_tex = position.xz;
}
