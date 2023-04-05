#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texture;
layout(location = 2) in vec3 normal;

layout(set = 0, binding = 0) uniform world_information {
  mat4 projection;
  mat4 view;
  mat4 model;
} world;

void main(void) {

  gl_Position =
    world.projection * inverse(world.view) * world.model * vec4(position, 1);
}
