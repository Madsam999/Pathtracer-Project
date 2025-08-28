#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D image_out;

uniform vec4 ourColor;

const float PI = 3.1415926535897932384626433832795;
const float EPSILON = 0.00001;
const float MAX_UINT = 4294967295.0;

void main() {
    ivec2 global_id = ivec2(gl_GlobalInvocationID.xy);
    ivec2 image_size = imageSize(image_out);
    vec2 normalized_coords = vec2(global_id) / vec2(image_size);
    vec4 color = vec4(normalized_coords, 1.0f, 1.0f);

    imageStore(image_out, global_id, ourColor);
}