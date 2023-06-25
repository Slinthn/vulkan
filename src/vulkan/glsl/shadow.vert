#version 450

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texture;

#include "common.glsl"

void main(
    void
){
    gl_Position = world.camera_projection
        * inverse(world.camera_view)
        * index.model
        * vec4(position.x, -position.y, position.z, 1);
}
