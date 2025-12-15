#version 430

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// Color buffers
layout(binding = 0, rgba32f) uniform readonly image2D ColorIn;
layout(binding = 1, rgba32f) uniform writeonly image2D ColorOut;

layout(binding = 2, r32f) uniform readonly image2D VarianceIn;
layout(binding = 3, r32f) uniform writeonly image2D VarianceOut;

layout(binding = 4, rgba32f) uniform readonly image2D NormalTexture;
layout(binding = 5, r32f) uniform readonly image2D DepthTexture;

uniform int stepSize;

const float PHI_COLOR = 4.f;
const float PHI_NORMAL = 128.f;
const float PHI_DEPTH = 1.f;

// 5x5 B-Spline kernel (approximated Gaussian Weights)
const float kernel[25] = float[](
    1.0/256.0, 1.0/64.0, 3.0/128.0, 1.0/64.0, 1.0/256.0,
    1.0/64.0,  1.0/16.0, 3.0/32.0,  1.0/16.0, 1.0/64.0,
    3.0/128.0, 3.0/32.0, 9.0/64.0,  3.0/32.0, 3.0/128.0,
    1.0/64.0,  1.0/16.0, 3.0/32.0,  1.0/16.0, 1.0/64.0,
    1.0/256.0, 1.0/64.0, 3.0/128.0, 1.0/64.0, 1.0/256.0
);

float luminance(vec3 c) {
    return dot(c, vec3(0.2126, 0.7152, 0.0722));
}

void main() {
    ivec2 pixelPos = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(ColorIn);

    vec4 inColorRead    = imageLoad(ColorIn, pixelPos);
    vec3 centerColor = inColorRead.rgb;
    float accumulationSave = inColorRead.a;
    float centerVariance = imageLoad(VarianceIn, pixelPos).r;
    vec3  centerNormal = imageLoad(NormalTexture, pixelPos).xyz;
    float centerDepth = imageLoad(DepthTexture, pixelPos).r;
    float centerLum = luminance(centerColor);

    float ddx = imageLoad(DepthTexture, pixelPos + ivec2(1, 0)).r - centerDepth;
    float ddy = imageLoad(DepthTexture, pixelPos + ivec2(0, 1)).r - centerDepth;
    float centerDepthGradient = abs(ddx) + abs(ddy) + 1e-6;

    vec3 sumColor = vec3(0.f);
    float sumVariance = 0.f;
    float sumWeight = 0.f;

    // 5x5 filter
    for(int y = -2; y <= 2; y++) {
        for(int x = -2; x <= 2; x++) {
            ivec2 offset = ivec2(x, y) * stepSize;
            ivec2 samplePos = pixelPos + offset;

            // Boundary check to stay within image
            if(samplePos.x < 0 || samplePos.y < 0 || samplePos.x >= size.x || samplePos.y >= size.y) {
                continue;
            }

            vec3 sampleColor = imageLoad(ColorIn, samplePos).rgb;
            float sampleVariance = imageLoad(VarianceIn, samplePos).r;
            vec3 sampleNormal = imageLoad(NormalTexture, samplePos).rgb;
            float sampleDepth = imageLoad(DepthTexture, samplePos).r;
            float sampleLuminance = luminance(sampleColor);

            float weightDepth = abs(centerDepth - sampleDepth) / centerDepthGradient;
            weightDepth = exp(-weightDepth * PHI_DEPTH);

            float dotP = dot(centerNormal, sampleNormal);

            float weightNormal = pow(max(0.f, dotP), PHI_NORMAL);

            float weightLuminance = abs(centerLum - sampleLuminance) / (PHI_COLOR * sqrt(max(0.f, centerVariance)) + 1e-6);
            weightLuminance = exp(-weightLuminance);

            float kernelWeight = kernel[(y + 2) * 5 + (x + 2)];

            float w = weightLuminance * weightDepth * weightNormal;

            float finalWeight = w * kernelWeight;

            sumColor += sampleColor * finalWeight;
            sumVariance += sampleVariance * (finalWeight * finalWeight);
            sumWeight += finalWeight;
        }
    }

    vec3 finalColor = sumColor / sumWeight;
    float finalVariance = sumVariance / (sumWeight * sumWeight);

    if (any(isnan(finalColor)) || any(isinf(finalColor))) {
        finalColor = vec3(0.0); // Output black instead of crashing/poisoning history
    }

    imageStore(ColorOut, pixelPos, vec4(finalColor, accumulationSave));
    imageStore(VarianceOut, pixelPos, vec4(finalVariance, 0.f, 0.f, 0.f));
}