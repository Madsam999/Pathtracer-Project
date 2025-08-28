#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <filesystem>

#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include "Camera.h"
#include "iostream"

#include "glm/vec3.hpp"
#include "glm/glm.hpp"
#include <GLFW/glfw3.h>

#include "Sphere.h"
#include "Ray.h"
#include "ComputeShader.h"
#include "Shader.h"

#include "string"
#include "vector"
#include "memory"
#include "algorithm"
#include "Engine.h"
#include "Scene.h"

struct RayTracerSettings {
    int maxRayBounce;
    int rayPerPixel;
};

void convertDataToPPM(const char *filename, int width, int height, const std::vector<glm::vec3> data) {

    std::vector<unsigned char> image;
    image.reserve(width * height * 3);

    for (const auto &color : data) {
        glm::vec3 correctedColor = glm::clamp(color, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
        image.push_back(static_cast<unsigned char>(correctedColor.x * 255.999));
        image.push_back(static_cast<unsigned char>(correctedColor.y * 255.999));
        image.push_back(static_cast<unsigned char>(correctedColor.z * 255.999));
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    // Write the header
    fprintf(file, "P6\n"); // P6 indicates a binary PPM file
    fprintf(file, "%d %d\n", width, height);
    fprintf(file, "255\n"); // Maximum color value


    fwrite(image.data(), 1, image.size(), file);

    fclose(file);
}

void writePPM(const char *filename, int width, int height, const std::vector<unsigned char> image) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    // Write the header
    fprintf(file, "P6\n"); // P6 indicates a binary PPM file
    fprintf(file, "%d %d\n", width, height);
    fprintf(file, "255\n"); // Maximum color value


    fwrite(image.data(), 1, image.size(), file);

    fclose(file);
}

void showTime(double time, std::string title) {
    std::cout << title << " (s): " << time << " seconds" << std::endl;
    std::cout << title << " (min): " << time / 60 << " minutes" << std::endl;
    std::cout << title << " (h): " << time / 3600 << " hours" << std::endl;
}

bool initializeGLFW(int width, int height, const char *title, GLFWwindow **window) {
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    *window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!*window) {
        std::cout << "Failed to open GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    return true;
}

int main() {
    std::cout << "Hello World!\n";

    int width = 1920;
    int height = 1080;
    int numSamples = 1;

    Camera camera(60.0f, glm::vec3(0, 0, 0), width, height, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    Scene scene(1, 2, &camera);

    Engine engine("Hello World", width, height);

    engine.createComputeShader("Shaders/test_shader.comp.glsl");
    engine.createShaderProgram("Shaders/test_vert.vert", "Shaders/test_frag.frag");

    engine.RenderSingleFrame(width, height, 100, 50, "test");

    engine.run();

    //std::vector<glm::vec3> image = scene.renderTest();
    //convertDataToPPM("renderTest.ppm", width, height, image);

    return 0;
}