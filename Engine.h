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



class Engine {
public:
    Engine(std::string title, int width, int height);
    ~Engine();
    void run();
    void createComputeShader(std::string shaderName);
    void createShaderProgram(std::string vertexShaderName, std::string fragmentShaderName);

    void RenderSingleFrame(std::string outputName, GLuint texture);
private:
    GLFWwindow* window;
    int width, height;
    std::string title;
    Scene scene;
    Camera camera;

    Shader shader;
    ComputeShader raytracer;
};



#endif //ENGINE_H
