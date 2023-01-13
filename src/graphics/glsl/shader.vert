#version 450

vec2 positions[3] = vec2[](
  vec2(0, -0.5),
  vec2(0.5, 0.5),
  vec2(-0.5, 0.5)
);

vec3 colours[3] = vec3[](
  vec3(1, 0, 0),
  vec3(0, 1, 0),
  vec3(0, 0, 1)
);

layout(location = 0) out vec3 fragment_colour;

void main(void) {
  gl_Position = vec4(positions[gl_VertexIndex], 0, 1);
  fragment_colour = colours[gl_VertexIndex];
}
