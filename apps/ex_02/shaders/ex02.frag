#version 410 core

layout (location=0) in vec3 colour;
layout (location=1) in vec3 world_pos;

layout (location=0) out vec4 frag_colour;

uniform float sigma_a;
uniform vec3 scatter_colour;
uniform vec3 cam_pos;
uniform float step_size;
uniform vec3 light_position;
uniform vec3 light_colour;

bool intersects_unit_sphere(vec3 ray_dir, vec3 cam_pos, out float t0, out float t1){
    vec3 L = -cam_pos;// Since the sphere is at the origin, L is just -cam_pos
    float a = dot(ray_dir, ray_dir);// Should be 1 if ray_dir is normalized
    float b = 2.0 * dot(L, ray_dir);
    float c = dot(L, L) - 1.0;// Sphere radius is 1, so (r^2 = 1)

    float delta = b*b - 4.0*a*c;
    if (delta < 0) return false;// No intersection

    float sqrt_delta = sqrt(delta);
    float tt0 = (-b - sqrt_delta) / (2.0 * a);
    float tt1 = (-b + sqrt_delta) / (2.0 * a);

    t0 = min(tt0, tt1);
    t1 = max(tt0, tt1);
    return true;// Intersection occurs
}

vec3 get_light_contribution(vec3 start_pos, vec3 light_position, vec3 light_colour, vec3 scatter_colour, float sigma_a){
    float t0 = 0;
    float t1 = 0;
    vec3 ray_dir = normalize(start_pos-light_position);
    intersects_unit_sphere(ray_dir, start_pos, t0, t1);
    float distance = t1;// t0 is implicitly 0
    float T = exp(-distance * sigma_a);
    return  (T * light_colour) + ((1 - T) * scatter_colour);
}

void main() {
    // Ray direction
    vec3 ray_dir = normalize(cam_pos-world_pos);
    float t0 = 0;
    float t1 = 0;
    bool intersects = intersects_unit_sphere(ray_dir, cam_pos, t0, t1);
    float distance = intersects ? t1 - t0 : 0;

    int num_steps = int(round(distance / step_size));
    vec3 bleed_through_light = colour;
    for (float i=0.5f; i < num_steps; i+=1.0f) {
        vec3 start_pos = cam_pos + (t0 + (i * step_size)) * ray_dir;
//        vec3 lc = get_light_contribution(start_pos, light_position, light_colour, scatter_colour, sigma_a);
        float T = exp(-step_size * sigma_a);
        bleed_through_light = (T * bleed_through_light) + ((1 - T) * scatter_colour);
//        bleed_through_light += lc;
    }
    frag_colour = vec4(bleed_through_light, 1);
}

