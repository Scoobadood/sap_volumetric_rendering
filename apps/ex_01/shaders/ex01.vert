#version 410 core

layout(location=0) in vec3 pos;
layout(location=1) in vec3 col;

layout (location=0) out vec3 colour;
layout (location=1) out vec3 world_pos;

uniform float aspect_ratio;
uniform vec3 wall_colour;

mat4 make_ortho() {
    float l = -3.0f*aspect_ratio;
    float r =  3.0f*aspect_ratio;
    float b = -3.0f;
    float t =  3.0f;
    float n =  0.01f;
    float f =  100.0f;

    mat4 projection = mat4(
        vec4(2.0/(r-l),     0.0,          0.0,         0.0),
        vec4(0.0,           2.0/(t-b),    0.0,         0.0),
        vec4(0.0,           0.0,         -2.0/(f-n),   0.0),
        vec4(-(r+l)/(r-l), -(t+b)/(t-b), -(f+n)/(f-n), 1.0)
    );
    return projection;
}

void main() {
    mat4 project = make_ortho();
    gl_Position = project * vec4(pos, 1.0);
    colour = wall_colour;
    world_pos = pos;
}
