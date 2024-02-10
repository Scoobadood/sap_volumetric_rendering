#version 410 core

layout(location=0) in vec3 pos;

layout (location=0) out vec3 colour;
layout (location=1) out vec3 world_pos;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform vec3 wall_colour;

void main() {
    gl_Position = projection * view * model * vec4(pos, 1.0);
    colour = wall_colour;
    world_pos = (model * vec4(pos, 1)).xyz;
}
