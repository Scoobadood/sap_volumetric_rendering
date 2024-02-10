#version 410 core

layout (location=0) in vec3 colour;
layout (location=1) in vec3 world_pos;

layout (location=0) out vec4 frag_colour;

uniform float sigma_a;
uniform vec3 scatter_colour;
uniform vec3 cam_pos;

bool intersects_unit_sphere(vec3 ray_dir, vec3 cam_pos, out float t0, out float t1){
    vec3 L = -cam_pos; // Since the sphere is at the origin, L is just -cam_pos
    float a = dot(ray_dir, ray_dir); // Should be 1 if ray_dir is normalized
    float b = 2.0 * dot(L, ray_dir);
    float c = dot(L, L) - 1.0; // Sphere radius is 1, so (r^2 = 1)

    float delta = b*b - 4.0*a*c;
    if (delta < 0) return false; // No intersection

    float sqrt_delta = sqrt(delta);
    float tt0 = (-b - sqrt_delta) / (2.0 * a);
    float tt1 = (-b + sqrt_delta) / (2.0 * a);

    t0 = min(tt0,tt1);
    t1 = max(tt0,tt1);
    return true; // Intersection occurs
}

void main() {
    // Ray direction
    vec3 ray_dir = normalize(cam_pos-world_pos);
    float t0 = 0;
    float t1 = 0;
    bool i = intersects_unit_sphere(ray_dir, cam_pos, t0, t1);
    float distance = 0;
    if( i ) distance = t1 - t0;

    float T = exp(-distance * sigma_a);
    vec3 background_color_through_volume = (T * colour) + ((1 - T) * scatter_colour);
    frag_colour = vec4(background_color_through_volume, 1);
}