#version 450

layout(location=0) in vec2 tex;
layout(location=1) in vec4 shadow_tex;
layout(location=2) in vec4 o_pos;

layout(location=0) out vec4 final_colour;

layout(set=1, binding=0) uniform sampler2D samp;
layout(set=2, binding=0) uniform sampler2D shadow;

#include "common.glsl"

void main(
    void
){

/*
    TODO: fuck the shadows for now. they are so fucked

    vec3 proj_coords = shadow_tex.xyz / shadow_tex.w;
    proj_coords = proj_coords * 0.5 + 0.5;
    
    float shade = 1.0f;

    if (proj_coords.x > 0 && proj_coords.x < 1 && proj_coords.y > 0
        && proj_coords.y < 1) {
        if (proj_coords.z - 0.05 > texture(shadow, proj_coords.xy).r) {
            shade = 0.2f;
        }
    }

    final_colour = texture(samp, tex) * shade;
*/

    final_colour = vec4(o_pos.xyz / 100.0f, 1);

    /*if ((index.flags & OBJECT_FLAG_TEXTURED) != 0) {
        final_colour = texture(samp, tex);
    }*/

    if ((index.flags & OBJECT_FLAG_COLOURED) > 0) {
        vec3 colour = index.colour.xyz * index.colour.w;
        final_colour += vec4(colour, 0);
    }
}
