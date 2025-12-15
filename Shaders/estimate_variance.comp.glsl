#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// Current frame Color (C_i) : firstMoment = firstMoment.rgb (or .xyz);
layout(binding = 0, rgba32f) uniform readonly image2D firstMoments;
// Current frame Color^2 (C_i^2) : secondMoment = secondMoments.rgb (or .xyz);
layout(binding = 1, rgba32f) uniform readonly image2D secondMoments;

// Current frame depths (t_i) : depth = depthTexture.x (or .r);
layout(binding = 2, r32f) uniform readonly image2D depthTexture;
// Current frame normals (N_i) : Normal = normalTexture.xyz (or .rgb);
layout(binding = 3, rgba32f) uniform readonly image2D normalTexture;

layout(binding = 4, r32f) uniform writeonly image2D varianceTexture;

// Constants:
const int FILTER_KERNEL_SIZE = 3; // Gives a 7x7 filter
const float PHI_NORMAL = 128.f;
const float PHI_DEPTH = 1.f;

void main() {
    ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(firstMoments);

    vec3 centerNormal = imageLoad(normalTexture, pixelPos).xyz;
    float centerDepth = imageLoad(depthTexture, pixelPos).x;

    // Find gradient of current pixel
    float ddx = imageLoad(depthTexture, pixelPos + ivec2(1, 0)).r - centerDepth;
    float ddy = imageLoad(depthTexture, pixelPos + ivec2(0, 1)).r - centerDepth;
    float centerDepthGradient = abs(ddx) - abs(ddy) + 1e-6;

    float sumWeight = 0.f;

    vec3 sumMoments1 = vec3(0.f);
    vec3 sumMoments2 = vec3(0.f);

    for(int y = -FILTER_KERNEL_SIZE; y <= FILTER_KERNEL_SIZE; y++) {
        for(int x = -FILTER_KERNEL_SIZE; x <= FILTER_KERNEL_SIZE; x++) {
            ivec2 samplePos = pixelPos + ivec2(x, y);

            // Boundary check to stay within image
            if(samplePos.x < 0 || samplePos.y < 0 || samplePos.x >= size.x || samplePos.y >= size.y) {
                continue;
            }

            vec3 sampleNormal = imageLoad(normalTexture, samplePos).xyz;
            float sampleDepth = imageLoad(depthTexture, samplePos).x;

            float weightNormal = pow(max(0.f, dot(centerNormal, sampleNormal)), PHI_NORMAL);

            float weightDepth = (abs(centerDepth - sampleDepth) / centerDepthGradient);
            weightDepth = exp(-weightDepth * PHI_DEPTH);

            float w = weightNormal * weightDepth;

            if(w > 1e-4) {
                vec3 m1 = imageLoad(firstMoments, samplePos).rgb;
                vec3 m2 = imageLoad(secondMoments, samplePos).rgb;

                sumMoments1 += m1 * w;
                sumMoments2 += m2 * w;
                sumWeight += w;
            }
        }
    }

    sumMoments1 /= sumWeight;
    sumMoments2 /= sumWeight;

    // V = E[X^2] - E[X]^2
    vec3 varianceRGB = abs(sumMoments2 - sumMoments1 * sumMoments1);

    float varianceFinal = max(varianceRGB.r, max(varianceRGB.g, varianceRGB.b));

    imageStore(varianceTexture, pixelPos, vec4(varianceFinal, 0.f, 0.f, 0.f));
}