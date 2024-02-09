#version 410 core

layout (location=0) in vec3 colour;
layout (location=1) in vec3 world_pos;

layout (location=0) out vec4 frag_colour;

uniform float sigma_a;
uniform vec3 volume_colour;

void main() {
    // Update for sphere with radius 1 at origin
    float x = min(max(-1,world_pos.x), 1);
    float y = min(max(-1,world_pos.y), 1);
    float z = sqrt(1 - (x*x+y*y));

    float distance = z<1? (2*z):0;

    float T = exp(-distance * sigma_a);
    vec3 background_color_through_volume = T * colour + (1 - T) * volume_colour;
    frag_colour = vec4(background_color_through_volume, 1);
}