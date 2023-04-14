#version 450

layout(location=0) in vec2 tex;
layout(location=1) in vec4 shadow_tex;

layout(location=0) out vec4 final_colour;

layout(set=1, binding=0) uniform sampler2D samp;
layout(set=2, binding=0) uniform sampler2D shadow;

void main(
    void
){
    vec3 proj_coords = shadow_tex.xyz / shadow_tex.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    float shade = 1.0f;

    if (proj_coords.x > 0 && proj_coords.x < 1 && proj_coords.y > 0
        && proj_coords.y < 1) {
        if (proj_coords.z - 0.2 > texture(shadow, proj_coords.xy).r) {
            shade = 0.2f;
        }
    }
    final_colour = texture(samp, tex) * shade;
}
