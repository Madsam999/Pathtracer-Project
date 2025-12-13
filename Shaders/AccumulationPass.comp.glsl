#version 430 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

uniform sampler2D NoisyColorTexture;
uniform sampler2D MotionVectorTexture;
uniform sampler2D CurrentDepthTexture;
uniform sampler2D HistoryColorTexture;
uniform sampler2D HistoryDepthTexture;
uniform sampler2D HistoryNormalTexture;
uniform usampler2D HistoryMeshIDTexture;
uniform usampler2D CurrentMeshIDTexture;
uniform sampler2D CurrentNormalTexture;

layout(binding = 0, rgba32f) uniform image2D denoisedOutput;
layout(binding = 1, rgba32f) uniform image2D firstMomentOutput;
layout(binding = 2, rgba32f) uniform image2D secondMomentOutput;

uniform int FrameCount;

const float ALPHA_MAX = 0.95; // Max accumulation factor (to prevent infinite accumulation)
const float DEPTH_THRESHOLD = 0.05; // Depth validation epsilon
const float NORMAL_THRESHOLD = 0.90; // Normal validation threshold (dot product)
const float EPSILON = 0.001;
const float MAX_ACCUMULATION = 32.f;

void main() {
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 screenDim = imageSize(denoisedOutput);
    vec2 currentUV = (vec2(pixelCoords) + 0.5f) / vec2(screenDim);

    vec4 noisyColor = texture(NoisyColorTexture, currentUV);
    float currentDepth = texture(CurrentDepthTexture, currentUV).r;

    vec4 motionTexel = texture(MotionVectorTexture, currentUV);
    vec2 motionVector = motionTexel.rg;

    vec2 historyUV = currentUV - motionVector;

    bool isValid = true;

    // Simple image bound check
    if(historyUV.x < 0.f || historyUV.x > 1.f ||
        historyUV.y < 0.f || historyUV.y > 1.f) {
        isValid = false;
    }

    vec4 historyColor = vec4(0.f);
    float historyDepth = 0.f;
    vec3 historyNormal = vec3(0.f);
    int historyMeshID = 0;

    if(isValid) {
        historyColor = texture(HistoryColorTexture, historyUV);
        historyDepth = texture(HistoryDepthTexture, historyUV).r;
        historyNormal = normalize(texture(HistoryNormalTexture, historyUV).rgb);
        historyMeshID = int(texture(HistoryMeshIDTexture, historyUV).r);

        if(abs(currentDepth - historyDepth) > DEPTH_THRESHOLD) {
            isValid = false;
        }

        vec3 currentNormal = normalize(texture(CurrentNormalTexture, currentUV).rgb);
        if(length(currentNormal) < EPSILON && length(historyNormal) < EPSILON) {
            isValid = true;
        }
        else if(dot(currentNormal, historyNormal) < NORMAL_THRESHOLD) {
            isValid = false;
        }

        int currentMeshID = int(texture(CurrentMeshIDTexture, currentUV).r);
        if(currentMeshID != historyMeshID) {
            isValid = false;
        }

    }

    vec3 finalColor = noisyColor.rgb;
    float currentAccumulationCount = 1.f;
    if(isValid) {
        float oldAccumulationCount = historyColor.a;
        float maxCount = 1024.f;
        float clampedOldCount = min(oldAccumulationCount, maxCount);
        float alpha = 1.f / (clampedOldCount + 1);
        //finalColor = vec3(1, 1, 1);
        finalColor = mix(historyColor.rgb, noisyColor.rgb, alpha);

        currentAccumulationCount = oldAccumulationCount + 1;
    }
    vec3 colorSquared = finalColor * finalColor;



    imageStore(firstMomentOutput, pixelCoords, vec4(finalColor, 1.f));
    imageStore(secondMomentOutput, pixelCoords, vec4(colorSquared, 1.f));

    imageStore(denoisedOutput, pixelCoords, vec4(finalColor.rgb, currentAccumulationCount));
}