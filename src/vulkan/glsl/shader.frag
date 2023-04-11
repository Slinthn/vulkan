#version 450

layout(location=0) in vec2 tex;

layout(location=0) out vec4 final_colour;

layout(set=1, binding=0) uniform sampler2D samp;

void main(
    void
){
    final_colour = texture(samp, tex);
}
