#version 450

layout(location = 0) in vec3 pos;

layout(location = 0) out vec4 final_colour;

void main(void)
{
    final_colour = vec4(pos / 10, 1);
}
