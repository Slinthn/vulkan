#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texture;
layout(location = 2) in vec3 normal;

void main(void) {
  gl_Position = vec4(position, 1);
}
