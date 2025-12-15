//
// Created by Samuel on 11/24/2025.
//

#ifndef SVGFDENOISER_H
#define SVGFDENOISER_H
#include <vector>

#include "ComputeShader.h"
#include "Shader.h"

class SVGFDenoiser {
public:
    SVGFDenoiser(int width, int height) : width(width), height(height) {
    };

    GLuint getNoisyTexture() {
        return noisyColorTexture;
    }
    GLuint getNormalTexture(int currentFrameIndex) {
        return normalTextures[currentFrameIndex];
    }
    GLuint getDepthTexture(int currentFrameIndex) {
        return depthTextures[currentFrameIndex];
    }
    GLuint getMotionVectorTexture() {
        return motionVectorTexture;
    }
    GLuint getDenoisedTexture(int currentFrameIndex) {
        return denoisedTextures[currentFrameIndex];
    }
    GLuint getMeshIdTexture() {
        return meshIDTexture;
    }

    void bindTexture(int currentFrameIndex);
    void initializeRessources();

    void copyBuffersToHistory(int currentFrameIndex, int historyFrameIndex);
    void copyNoisyToHistory(int writeIndex);
    void initializeMoments(int writeIndex);

    void accumulationPass(int currentFrameIndex, int historyFrameIndex, int frameCnt);

    void varianceEstimatePass(int currentFrameIndex);
    void atrousFilterPass(int& currentFrameIndex, int& historyFrameIndex);

private:
    int width, height;

    // Stores previous and current denoised frame
    std::vector<GLuint> denoisedTextures;
    // Store previous and current frame first raw moment (mu_1')
    std::vector<GLuint> firstRawMomentTextures;
    // Stores previous and current frame second raw moment (mu_2')
    std::vector<GLuint> secondRawMomentTextures;

    // Stores previous and current frame depth
    std::vector<GLuint> depthTextures;
    // Stores previous and current frame normals
    std::vector<GLuint> normalTextures;
    // Stores previous and current frame meshID
    std::vector<GLuint> meshIDTextures;

    // Stores the intermediate results of A-Trous wavelet transform
    std::vector<GLuint> intermediateTextures;

    std::vector<GLuint> varianceTextures;
    GLuint noisyColorTexture;
    GLuint depthTexture;
    GLuint normalTexture;
    GLuint meshIDTexture;
    GLuint motionVectorTexture;
    GLuint rawSecondMomentsTexture;

    GLuint createTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum type, GLenum param);

    ComputeShader initializationShader;
    ComputeShader accumulationPassShader;
    ComputeShader variancePassShader;
    ComputeShader atrousPassShader;
};



#endif //SVGFDENOISER_H
