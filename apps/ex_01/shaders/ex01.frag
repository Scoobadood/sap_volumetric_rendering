#version 410 core

layout (location=0) in vec3 colour;

layout (location=0) out vec4 frag_colour;

void main() {
    frag_colour = vec4(colour, 1);
}