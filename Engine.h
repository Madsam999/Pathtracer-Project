//
// Created by Samuel on 12/4/2025.
//

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image_write.h"
#include <string>
#include <algorithm>
#include <iostream>
#include <ostream>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

#include "Shader.h"
#include "ComputeShader.h"
#include "Scene.h"
#include "CameraController.h"
#include "SVGFDenoiser.h"

#ifndef ENGINE_H
#define ENGINE_H

struct alignas(16) MaterialInfo {
    glm::vec4 color, emissionColor; // 16-31
    float emissionStrength, specular; // 36-39
    float padding2[2]; // 40-47
};

struct alignas(16) SphereInfo {
    glm::mat4 transform, invTransorm; // 64-127
    glm::mat4 prevTransform, prevInverseTransform;
    MaterialInfo mat; // 128 - 175
    glm::uvec4 objectID;
};

struct alignas(16) TriangleInfo {
    glm::vec4 positionA, positionB, positionC;
    glm::vec4 normalA, normalB, normalC;
};

struct alignas(16) MeshInfo {
    glm::mat4 transform, invTransform;
    glm::mat4 prevTransform, prevInverseTransform;
    glm::vec4 boundsMin, boundsMax;
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

inline void resetAccumulation(unsigned int textureID) {
    const float zero[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearTexImage(textureID, 0, GL_RGBA, GL_FLOAT, &zero);
}

class Engine {
public:
    Engine(const std::string& title, const int width, const int height) :
        denoiser(width, height)
    {
        this->width = width;
        this->height = height;

        createGLWFContext(title.c_str());
        createImGuiContext();

        denoiser.initializeRessources();

        debugMode = DebugMode::ACCUMULATION_TEXTURE;
    }
    ~Engine() {
        std::cout << "Engine closing" << std::endl;
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void bindScene(Scene* scene) {
        this->scene = scene;
    }
    void bindCamera(Camera* camera) {
        this->camera = camera;
        cameraController = std::make_unique<CameraController>(*camera, 0.05f, 0.1f);
    }

    void createComputeShader(std::string shaderName);
    void createShaderProgram(std::string vertexShaderName, std::string fragmentShaderName);

    void run();
private:
    // Member variables
    GLFWwindow* window;
    int width, height;

    Scene* scene;
    Camera* camera;
    std::unique_ptr<CameraController> cameraController;

    Shader shader;
    ComputeShader raytracer;
    SVGFDenoiser denoiser;

    DebugMode debugMode;

    GLuint meshSSBO, triangleSSBO, sphereSSBO;

    // Private functions
    void createGLWFContext(const char* title);
    void createImGuiContext();

    void initializeSSBO();
    void initializeSphereSSBO();
    void initializeMeshSSBO();
    void updateSSBO();

    void updateMovingSphere(int sphereIndex);

    unsigned int createRenderTarget();

    void raytracePass(int frame, int currentFrame, int historyFrame);
    void accumulationPass(int frame, int currentFrame, int historyFrame);
    void renderToScreen(int currentFrame, unsigned int quadVAO);
    void renderGUI();

    void handleInputEvents();
};



#endif //ENGINE_H
