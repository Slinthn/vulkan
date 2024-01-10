#version 450

layout(location=0) in vec3 position;

layout(set=0, binding=0) uniform world_info {
    mat4 projection, view;
} w;

void main(void) {
    gl_Position = w.projection * w.view * vec4(position, 1);
}

