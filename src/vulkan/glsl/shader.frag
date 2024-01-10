#version 450

layout(location=0) in vec2 tex;
layout(location=1) in vec4 shadow_tex;
layout(location=2) in vec4 o_pos;
layout(location=3) in vec3 o_norm;
layout(location=4) in vec2 o_tex;

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


    /*if ((index.flags & OBJECT_FLAG_TEXTURED) != 0) {
        final_colour = texture(samp, tex);
    }*/

    if ((index.flags & OBJECT_FLAG_COLOURED) > 0) {
        vec3 colour = index.colour.xyz * index.colour.w;
        final_colour += vec4(colour, 0);
    }

    vec4 c_grass = vec4(56 / 255.0f, 128 / 255.0f, 4 / 255.0f, 1);
    vec4 c_rock = vec4(90 / 255.0f, 102 / 255.0f, 89 / 255.0f, 1);

    vec3 light = normalize(vec3(1, 1, -1));
    float d = dot(light, o_norm);
    d = max(d * 1.5f, 0.3f);

    vec3 up = vec3(0, 1, 0);
    float up_d = dot(up, o_norm);

    if (up_d > 0.5)
        up_d = min(up_d + 0.1f, 1);
    else
        up_d = max(up_d - 0.1f, 0);

    vec4 surf_col = up_d * c_grass + (1 - up_d) * c_rock;

    float snow = o_pos.y / 20;
    snow = min(snow, 0.5);
    surf_col += vec4(snow);

    final_colour = surf_col * d;
}

