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

enum ComputeType {
    RAYTRACING = 0,
    ACCUMULATE = 1
};



class Engine {
public:
    Engine(std::string title, int width, int height);
    ~Engine();
    void run();
    void createComputeShader(std::string shaderName, ComputeType type);
    void createShaderProgram(std::string vertexShaderName, std::string fragmentShaderName);

    void RenderSingleFrame(int width, int height, int rpp, int mrb, std::string outputName);
private:
    GLFWwindow* window;
    int width, height;
    std::string title;
    Scene scene;
    Camera camera;

    Shader shader;
    ComputeShader raytracer;
    ComputeShader accumulationShader;
};



#endif //ENGINE_H
