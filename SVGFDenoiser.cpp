//
// Created by Samuel on 11/24/2025.
//

#include "SVGFDenoiser.h"

void SVGFDenoiser::initializeRessources() {
    noisyColorTexture = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    /*
    depthTexture = createTexture(width, height, GL_R32F, GL_RED, GL_FLOAT);
    normalTexture = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    meshIDTexture = createTexture(width, height, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT);
    */
    motionVectorTexture = createTexture(width, height, GL_RG16F, GL_RG, GL_FLOAT);

    denoisedTextures.resize(2);
    firstRawMomentTextures.resize(2);
    secondRawMomentTextures.resize(2);
    depthTextures.resize(2);
    normalTextures.resize(2);
    meshIDTextures.resize(2);

    for (int i = 0; i < 2; i++) {
        denoisedTextures[i] = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        firstRawMomentTextures[i] = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        secondRawMomentTextures[i] = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);

        depthTextures[i] = createTexture(width, height, GL_R32F, GL_RED, GL_FLOAT);
        normalTextures[i] = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        meshIDTextures[i] = createTexture(width, height, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT);
    }

    varianceTexture = createTexture(width, height, GL_R32F, GL_RED, GL_FLOAT);

    intermediateTextures.resize(2);
    for (int i = 0; i < 2; i++) {
        intermediateTextures[i] = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    }

    initializationShader = ComputeShader("./Shaders/initializationShader.comp.glsl");
    accumulationPassShader = ComputeShader("./Shaders/AccumulationPass.comp.glsl");
}


GLuint SVGFDenoiser::createTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum type) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (internalFormat == GL_R32F || internalFormat == GL_R32UI) {
        GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }

    return texture;
}

void SVGFDenoiser::bindTexture(int currentFrameIndex) {
    glBindImageTexture(0, noisyColorTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, depthTextures[currentFrameIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    glBindImageTexture(2, normalTextures[currentFrameIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(3, meshIDTextures[currentFrameIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32UI);
    glBindImageTexture(4, motionVectorTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
}

void SVGFDenoiser::copyBuffersToHistory(int currentFrameIndex, int historyFrameIndex) {
    glCopyImageSubData(
        depthTextures[currentFrameIndex], GL_TEXTURE_2D, 0, 0, 0, 0,
        depthTextures[historyFrameIndex], GL_TEXTURE_2D, 0, 0, 0, 0,
        width, height, 1
    );

    glCopyImageSubData(
        normalTextures[currentFrameIndex], GL_TEXTURE_2D, 0, 0, 0, 0,
        normalTextures[historyFrameIndex], GL_TEXTURE_2D, 0, 0, 0, 0,
        width, height, 1
    );

    // --- Copy Mesh ID (R32UI) ---
    glCopyImageSubData(
        meshIDTextures[currentFrameIndex], GL_TEXTURE_2D, 0, 0, 0, 0,
        meshIDTextures[historyFrameIndex], GL_TEXTURE_2D, 0, 0, 0, 0,
        width, height, 1
    );
}

void SVGFDenoiser::copyNoisyToHistory(int writeIndex) {
    glCopyImageSubData(
        noisyColorTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
        denoisedTextures[writeIndex], GL_TEXTURE_2D, 0, 0, 0, 0,
        width, height, 1
    );
}

void SVGFDenoiser::initializeMoments(int writeIndex) {
    initializationShader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noisyColorTexture);
    initializationShader.setInt("NoisyColorTexture", 0);

    glBindImageTexture(0, firstRawMomentTextures[writeIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    initializationShader.setInt("FirstMomentImage", 0);

    // Binding 1: Second Raw Moment (mu_2') - Stores the mean squared (color^2)
    glBindImageTexture(1, secondRawMomentTextures[writeIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    initializationShader.setInt("SecondMomentImage", 1);

    glDispatchCompute(ceil(width / 16.0), ceil(height / 16.0), 1);
}

void SVGFDenoiser::accumulationPass(int currentFrameIndex, int historyFrameIndex, int frameCnt) {
    accumulationPassShader.use();

    accumulationPassShader.setInt("FrameCount", frameCnt);

    // C_i
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noisyColorTexture);
    accumulationPassShader.setInt("NoisyColorTexture", 0);

    // V_i
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, motionVectorTexture);
    accumulationPassShader.setInt("MotionVectorTexture", 1);

    // Z_i
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthTextures[currentFrameIndex]);
    accumulationPassShader.setInt("CurrentDepthTexture", 2);

    // C_i-1
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, denoisedTextures[historyFrameIndex]);
    accumulationPassShader.setInt("HistoryColorTexture", 3);

    // Z_i-1
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, depthTextures[historyFrameIndex]);
    accumulationPassShader.setInt("HistoryDepthTexture", 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, normalTextures[historyFrameIndex]);
    accumulationPassShader.setInt("HistoryNormalTexture", 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, meshIDTextures[historyFrameIndex]);
    accumulationPassShader.setInt("HistoryMeshIDTexture", 6);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, meshIDTextures[currentFrameIndex]);
    accumulationPassShader.setInt("CurrentMeshIDTexture", 7);

    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, normalTextures[currentFrameIndex]);
    accumulationPassShader.setInt("CurrentNormalTexture", 8);

    glBindImageTexture(0, denoisedTextures[currentFrameIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, firstRawMomentTextures[currentFrameIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(2, secondRawMomentTextures[currentFrameIndex], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute(ceil(width / 16), ceil(height / 16), 1);
}




