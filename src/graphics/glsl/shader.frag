#version 450

layout(location = 0) in vec3 fragment_colour;
layout(location = 0) out vec4 final_colour;

void main(void) {
  final_colour = vec4(fragment_colour, 1);
}
