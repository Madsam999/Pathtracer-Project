//
// Created by Samuel on 11/24/2025.
//

#ifndef SVGFDENOISER_H
#define SVGFDENOISER_H
#include <memory>

#include "Shader.h"

#include <glm/glm.hpp>


class SVGFDenoiser_old {
public:

    struct Parameters {
        float alpha = 0.2;
        float sigma_z = 1.f;
        float sigma_n = 1.5f;
        float sigma_l = 4.f;
        int waveletLevels = 5;
    };

    SVGFDenoiser_old(int width, int height) : width(width), height(height) {};

    // Main execution function called once per frame by the Engine
    void processFrame(
        GLuint noisyColorTexture,          // Input: 1 spp Path-Traced Color
        GLuint gBufferFBO,                 // Input: FBO containing Depth, Normals, IDs
        GLuint motionVectorTexture,        // Input: Current frame Motion Vectors
        const glm::mat4& viewProjMatrix,   // Input: Current Frame ViewProj Matrix
        const glm::mat4& prevViewProjMatrix // Input: Prior Frame ViewProj Matrix
    );

    // Returns the final denoised image texture for display
    GLuint getFinalOutputTexture() const { return m_intermediateFBO[m_historyReadIndex].colorTexture; }
private:
    int width, height;

    std::unique_ptr<Shader> temporalAccumulationShader;
    std::unique_ptr<Shader> varianceEstimationShader;
    std::unique_ptr<Shader> waveletFilterShader;
    std::unique_ptr<Shader> spatialVarianceFilterShader;

    struct FrameBufferObject {
        GLuint fbo = 0;
        GLuint colorTexture = 0;
    };

    // Textures
    GLuint noisyColorTexture;
    GLuint depthTexture;
    GLuint normalTexture;
    GLuint meshIDTexture;
    GLuint motionVectorTexture;
    GLuint varianceTexture;

    std::vector<FrameBufferObject> historyFBOs;

    std::vector<GLuint> m_integratedColorTexture; // GL_RGB16F or GL_RGB32F (Size 2)
    std::vector<GLuint> m_integratedMoment1Texture; // GL_R32F (Size 2)
    std::vector<GLuint> m_integratedMoment2Texture; // GL_R32F (Size 2)
    std::vector<GLuint> m_historyDepthTexture; // GL_R32F (Size 2 - for consistency check)
    std::vector<GLuint> m_historyNormalTexture; // GL_RGB16F (Size 2 - for consistency check)
    std::vector<GLuint> m_historyMeshIDTexture; // GL_R32UI (Size 2 - for consistency check)
    std::vector<GLuint> m_intermediateColorTexture;

    int m_historyWriteIndex = 0; // Index for current frame's output
    int m_historyReadIndex = 1;  // Index for prior frame's input

    // --- 4. Intermediate Buffers ---
    // Used for the variance map and temporary wavelet passes
    FrameBufferObject m_varianceFBO;
    std::vector<FrameBufferObject> m_intermediateFBO; // Used for ping-ponging during wavelet passes (Size 2)

    void initializeResources();
    GLuint createTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum type);
    void destroyResources();
    void swapHistoryBuffers();

    void temporalAccumulationPass(GLuint noisyColor);
    void varianceEstimationPass(GLuint noisyColor);
    void spatialFilterPass();
};



#endif //SVGFDENOISER_H
