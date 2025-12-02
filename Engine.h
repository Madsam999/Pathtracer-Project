//
// Created by Samuel on 2025-08-23.
//

#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image_write.h"
#include <string>

#include "Shader.h"
#include "ComputeShader.h"
#include "Scene.h"
#include "CameraController.h"
#include "SVGFDenoiser.h"

struct alignas(16) MaterialInfo {
    glm::vec4 color; // 0-15
    glm::vec4 emissionColor; // 16-31
    float emissionStrength; // 32-35
    float specular; // 36-39
    float padding2[2]; // 40-47
};

struct alignas(16) SphereInfo {
    glm::mat4 transform; //0-63

    glm::mat4 invTransorm; // 64-127

    MaterialInfo mat; // 128 - 175
    glm::uvec4 objectID;
};

struct alignas(16) TriangleInfo {
    glm::vec4 positionA, positionB, positionC;
    glm::vec4 normalA, normalB, normalC;
};

struct alignas(16) MeshInfo {
    glm::mat4 transform;
    glm::mat4 invTransform;
    glm::vec4 boundsMin;
    glm::vec4 boundsMax;
    glm::uvec4 info;
    MaterialInfo material;
    glm::uvec4 objectID;
};

enum class DebugMode {
    NOISY_TEXTURE = 0,
    DEPTH_TEXTURE = 1,
    NORMAL_TEXTURE = 2,
    MOTION_TEXTURE = 3,
    ACCUMULATION_TEXTURE = 4,
    MESH_ID_TEXTURE = 5,
};


class Engine {
public:
    Engine(std::string title, int width, int height);
    ~Engine();
    void run();
    void createComputeShader(std::string shaderName);
    void createShaderProgram(std::string vertexShaderName, std::string fragmentShaderName);

    void RenderSingleFrame(std::string outputName, GLuint texture);

    void createSSBO();
    void sphereSSBO();
    void meshSSBO();

private:
    GLFWwindow* window;
    int width, height;
    std::string title;
    Scene scene;
    Camera camera;
    CameraController cameraController;

    Shader shader;
    ComputeShader raytracer;

    SVGFDenoiser denoiser;

    DebugMode debugMode;

    int currentFrameIndex;
    int historyFrameIndex;
    int frame;
    int currentDebugMode;
    GLuint textureToDisplay;
    std::string outputName = "Single_Frame";
    int screenShots = 0;;

    void HandleInputAndCameraUpdates();
    void RaytracePass();
    void ColdStart();
    void AccumulationPass();
    void FinalizeFrame();
    void SelectDebugTexture();
    void DisplayToScreen(unsigned int quadVAO, unsigned int quadVBO);
    void RenderGUI();
    void BeginFrame();
};



#endif //ENGINE_H
