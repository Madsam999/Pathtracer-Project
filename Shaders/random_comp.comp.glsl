#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D image_out;

struct Material {
    vec3 color;
};

struct Sphere {
    vec3 center;
    float radius;
    Material mat;
};

layout(std430, binding = 1) buffer layoutName {
    int numSpheres;
    Sphere spheres[];
};

const float PI = 3.1415926535897932384626433832795;
const float EPSILON = 0.00001;
const float MAX_UINT = 4294967295.0;

void main() {
    ivec2 global_id = ivec2(gl_GlobalInvocationID.xy);
    ivec2 image_size = imageSize(image_out);

    vec4 color = vec4(spheres[1].mat.color, 1);

    imageStore(image_out, global_id, color);
}