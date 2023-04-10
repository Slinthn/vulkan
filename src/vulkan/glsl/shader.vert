#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texture;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec3 pos;

layout(set = 0, binding = 0) uniform world_information {
    mat4 projection;
    mat4 view;
} world;

layout(set = 0, binding = 1) uniform object_information {
    mat4[100] model;
} object;

layout(push_constant) uniform object_index {
    int index;
} index;

/**
 *
 */
void main(void)
{
    gl_Position = world.projection * inverse(world.view)
            * object.model[index.index] * vec4(position, 1);
    pos = position;
}
