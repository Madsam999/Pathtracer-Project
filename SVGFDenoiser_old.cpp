//
// Created by Samuel on 11/24/2025.
//

#include "SVGFDenoiser_old.h"

void SVGFDenoiser_old::initializeResources() {
    noisyColorTexture = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    depthTexture = createTexture(width, height, GL_R32F, GL_RED, GL_FLOAT);
    normalTexture = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    meshIDTexture = createTexture(width, height, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT);
    motionVectorTexture = createTexture(width, height, GL_RG16F, GL_RG, GL_FLOAT);

    m_integratedColorTexture.resize(2);
    m_integratedMoment1Texture.resize(2);
    m_integratedMoment2Texture.resize(2);
    m_historyDepthTexture.resize(2);
    m_historyNormalTexture.resize(2);
    m_historyMeshIDTexture.resize(2);

    for (int i = 0; i < 2; i++) {
        m_integratedColorTexture[i] = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        m_integratedMoment1Texture[i] = createTexture(width, height, GL_R32F, GL_RED, GL_FLOAT);
        m_integratedMoment2Texture[i] = createTexture(width, height, GL_R32F, GL_RED, GL_FLOAT);

        m_historyDepthTexture[i] = createTexture(width, height, GL_R32F, GL_RED, GL_FLOAT);
        m_historyNormalTexture[i] = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
        m_historyMeshIDTexture[i] = createTexture(width, height, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT);
    }

    varianceTexture = createTexture(width, height, GL_R32F, GL_RED, GL_FLOAT);

    m_intermediateColorTexture.resize(2);
    for (int i = 0; i < 2; i++) {
        m_integratedColorTexture[i] = createTexture(width, height, GL_RGBA32F, GL_RGBA, GL_FLOAT);
    }
}

GLuint SVGFDenoiser_old::createTexture(int width, int height, GLenum internalFormat, GLenum format, GLenum type) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return texture;
}


