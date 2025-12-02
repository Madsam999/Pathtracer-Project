#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

uniform sampler2D NoisyColorTexture;

layout(binding = 0, rgba32f) uniform image2D FirstMomentImage;
layout(binding = 1, rgba32f) uniform image2D SecondMomentImage;

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imageSize = imageSize(FirstMomentImage);

    vec2 uv = vec2(pixelCoords) / vec2(imageSize);

    vec3 color = texture(NoisyColorTexture, uv).rgb;

    vec4 mu_1_prime = vec4(color, 1);

    vec3 colorSquared = color * color;

    vec4 mu_2_prime = vec4(colorSquared, 1);

    imageStore(FirstMomentImage, pixelCoords, mu_1_prime);
    imageStore(SecondMomentImage, pixelCoords, mu_2_prime);
}