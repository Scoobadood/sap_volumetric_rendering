#version 410 core

layout (location=0) in vec3 colour;
layout (location=1) in vec3 world_pos;

layout (location=0) out vec4 frag_colour;

uniform float sigma_a;
uniform float sigma_s;
uniform vec3 scatter_colour;
uniform vec3 cam_pos;
uniform vec3 sphere_pos;
uniform float sphere_radius;
uniform float step_size;
uniform vec3 light_position;
uniform vec3 light_colour;
uniform bool do_march_forwards;

bool solve_quadratic(float a, float b, float c, out float r0, out float r1) {
    float d = b * b - 4 * a * c;
    if (d < 0) return false;

    float rr0 = (0.5 * (-b +  sqrt(d))) / a;
    float rr1 = (0.5 * (-b -  sqrt(d))) / a;
    r0 = min(rr0, rr1);
    r1 = max(rr0, rr1);
    return true;
}

bool intersects_sphere(vec3 sphere_pos, float sphere_radius, vec3 ray_start, vec3 ray_dir, out float t0, out float t1){
    // - r0: ray origin
    // - rd: normalized ray direction
    // - s0: sphere center
    // - sr: sphere radius
    // - Returns distance from r0 to first intersecion with sphere,
    //   or -1.0 if no intersection.
    vec3 ray_start_to_sphere = ray_start - sphere_pos;
    float a = dot(ray_dir, ray_dir);
    float b = 2.0 * dot(ray_dir, ray_start_to_sphere);
    float c = dot(ray_start_to_sphere, ray_start_to_sphere) - (sphere_radius * sphere_radius);

    if (!solve_quadratic(a, b, c, t0, t1)) return false;

    if (t0 < 0) {
        if (t1 < 0) return false;
        else {
            t0 = 0;
        }
    }
    return true;
}


/*
 Consider 1D

 -3  -2  -1   0   1   2   3   4   5   6   7   8   9  10
 -+---+---+---+---+---+---+---+---+---+---+---+---+---+
          C       t0                                  t1
i                       2           1           0

 t0=2, t1 = 11
 ray_dir = (1,0,0)
 cam_pos = (-1,0,0)
 step_size = 3.0
 num_steps = (11-2)/3 = 3
 step_middle = (-1,0,0) + (11 - (1.5 * 1)) * (1,0,0)
             = (-1,0,0) + (9.5,0,0)

 */
vec3 march_backwards(float t0, float t1, vec3 ray_start, float num_steps, float ssize, vec3 ray_dir) {
    // Result from the light source
    vec3 result_colour = vec3(0, 0, 0);

    // Transmission
    float transparency = 1.0;

    // Iterate over all steps
    for (int i=0; i < num_steps; i++) {
        float t = t1 - ssize * (i + 0.5);
        vec3 sample_pos= ray_start + t * ray_dir;

        // Compute sample transparency using Beer's law
        float sample_transparency = exp(-step_size * (sigma_s + sigma_a));

        // Attenuate global transparency by sample transparency
        transparency *= sample_transparency;


        //
        // For each light
        vec3 light_ray_dir = normalize(light_position-sample_pos);
        float lt0 = 0;
        float lt1 = 0;
        // In-scattering. Find the distance traveled by light through
        // the volume to our sample point. Then apply Beer's law.
        intersects_sphere(sphere_pos, sphere_radius, sample_pos, light_ray_dir, lt0, lt1);
        // Expected that lt0 is 0.
        float light_attenuation = exp(-lt1 * (sigma_s + sigma_a));
        result_colour += light_colour * light_attenuation * sigma_s * 1.0 /*density */ * step_size;
        //
        //
        result_colour *= sample_transparency;
    }
    return colour* transparency + result_colour;
}

vec3 march_forwards(float t0, float t1, vec3 ray_start, float num_steps, float ssize,  vec3 ray_dir) {
    // Step 1: before we enter the ray-marching loop: initialize the overall transmission (transparency)
    // value to 1 and the result color variable to 0 (the variable that stores the volumetric object
    // color for the current camera ray):
    float transparency = 1.0;
    vec3 result_colour = vec3(0);

    // Step 2: for each iteration in the ray-marching loop:
    //      Compute in-scattering for the current sample: Li(x).
    for (int i=0; i < num_steps; i++) {
        float t = t0 + ssize * (i + 0.5);
        vec3 sample_pos= ray_start + t * ray_dir;

        float sample_attenuation = exp(-step_size * (sigma_a + sigma_s));
        transparency *= sample_attenuation;


        //
        // For each light
        vec3 light_ray_dir = normalize(light_position-sample_pos);
        float lt0 = 0;
        float lt1 = 0;
        // In-scattering. Find the distance traveled by light through
        // the volume to our sample point. Then apply Beer's law.
        intersects_sphere(sphere_pos, sphere_radius, sample_pos, light_ray_dir, lt0, lt1);
        // Expected that lt0 is 0.
        float light_attenuation = exp(-lt1 * (sigma_s + sigma_a));
        result_colour += transparency * light_colour * light_attenuation * sigma_s * 1.0 /*density */ * step_size;
        //
        //
    }
    return colour * transparency + result_colour;
}


void main() {
    // Ray direction
    vec3 ray_dir = normalize(world_pos-cam_pos);
    float t0 = 0;
    float t1 = 0;
    bool intersects = intersects_sphere(sphere_pos, sphere_radius, cam_pos, ray_dir, t0, t1);

    if( intersects) {
        float distance = t1 - t0;
        float num_steps = ceil(distance / step_size);
        float ssize = distance / num_steps;

        frag_colour =
        do_march_forwards
        ? vec4(march_forwards(t0, t1, cam_pos, num_steps, ssize, ray_dir), 1)
        : vec4(march_backwards(t0, t1, cam_pos, num_steps, ssize, ray_dir), 1);
    } else {
        frag_colour = vec4(colour,1);
    }
}