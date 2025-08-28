//
// Created by Samuel on 2025-08-23.
//

#include "Engine.h"

#include <algorithm>
#include <iostream>
#include <ostream>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

struct Mat {
    alignas(16) glm::vec3 color;
    float padding1;
    alignas(16)glm::vec3 emissionColor;
    float emissionStrength;
    float specular;
    float padding2[2];
};

struct SphereStruct {
    alignas(16) glm::vec3 center;
    float radius;
    Mat mat;
};

struct SphereBuffer {
    int numSpheres;
    SphereStruct* spheres;
};


Engine::Engine(std::string name, int width, int height)
    : width(width), height(height),
      camera(60.0f, glm::vec3(0, 0, 0), width, height, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
      scene(1, 2, &camera) {
    std::cout << "Creating window: " << title << std::endl;
    std::cout << "Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;
    if (!glfwInit()) {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLFW3");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD");
    }

    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

Engine::~Engine() {
    std::cout << "Engine closing" << std::endl;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Engine::run() {

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    RenderSingleFrame(1920, 1080, 5, 2, "Test");
    // Quad Vertices
    static float quadVerts[] = {
        // positions       // texture Coords
        -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
    };

    // Setup VAO and VBO for Quad
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    shader.setInt("tex", 0);

    // Texture Setup
    unsigned int raytracedTexture;

    glGenTextures(1, &raytracedTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, raytracedTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL); // Correct initialization
    glBindImageTexture(0, raytracedTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // Use WRITE_ONLY

    /*
    int rpp = 5;
    int mrb = 5;
    glm::vec3 RedSphereColor = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 LightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    bool DarkMode = false;
    */

    std::cout << "Size of Mat: " << sizeof(Mat) << " bytes" << std::endl;
    std::cout << "Size of float: " << sizeof(float) << " bytes" << std::endl;
    std::cout << "Size of int: " << sizeof(int) << " bytes" << std::endl;
    std::cout << "Size of glm::vec3: " << sizeof(glm::vec3) << " bytes" << std::endl;
    std::cout << "Size of SphereStruct: " << sizeof(SphereStruct) << " bytes" << std::endl;

    std::vector<Sphere> sceneSpheres = scene.getSpheres();
    std::vector<SphereStruct> spheres;

    for (int i = 0; i < sceneSpheres.size(); i++) {
        Sphere sphere = sceneSpheres[i];
        Material material = sphere.get_material();
        SphereStruct sphereData;
        sphereData.center = sphere.get_center();
        sphereData.radius = sphere.get_radius();
        Mat matData;
        matData.color = material.getColor();
        matData.emissionColor = material.getEmissionColor();
        matData.emissionStrength = material.getEmissionStrength();
        matData.specular = material.getSpecular();
        sphereData.mat = matData;

        spheres.push_back(sphereData);
    }

    int numSpheres = spheres.size();
    std::cout << "Number of spheres: " << numSpheres << std::endl;
    size_t header_size = 16;  // 4 (int) + 12 padding to align spheres[] to 16
    size_t sphere_data_size = sizeof(SphereStruct) * spheres.size();
    std::vector<char> ssbo_data(header_size + sphere_data_size);

    // write count
    std::memcpy(ssbo_data.data(), &numSpheres, sizeof(int));

    // zero the padding (optional but tidy)
    std::memset(ssbo_data.data() + sizeof(int), 0, header_size - sizeof(int));

    // write spheres starting at offset 16
    std::memcpy(ssbo_data.data() + header_size, spheres.data(), sphere_data_size);

    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_data.size(), ssbo_data.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    int rpp = 5;
    int mrb = 5;
    glm::vec3 RedSphereColor = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 LightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    bool DarkMode = false;

    while (!glfwWindowShouldClose(window)) {

        raytracer.use();

        raytracer.setInt("RayPerPixel", rpp);
        raytracer.setInt("MaxRayBounce", mrb);
        raytracer.setFloat3("ViewParams", camera.viewParams);
        raytracer.setFloat3("ViewCenter", camera.center);
        raytracer.setFloat44("View", camera.view);
        raytracer.setFloat3("RedSphereColor", RedSphereColor);
        raytracer.setFloat3("LightColor", LightColor);
        raytracer.setBool("DarkMode", DarkMode);

        raytracer.dispatch(ceil(width / 16.0), ceil(height / 16.0), 1, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();



        shader.use();
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);

        ImGui::Begin("Settings");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::SliderInt("RPP", &rpp, 1, 100);
        ImGui::SliderInt("Mrb", &mrb, 1, 100);
        ImGui::ColorEdit3("color", glm::value_ptr(RedSphereColor));
        ImGui::ColorEdit3("light", glm::value_ptr(LightColor));
        ImGui::SliderFloat("Cam x", &camera.center.x, -100, 100);
        ImGui::SliderFloat("Cam y", &camera.center.y, -100, 100);
        ImGui::SliderFloat("Cam z", &camera.center.z, -100, 100);
        ImGui::SliderFloat("Cam FOV", &camera.fov, 0, 360);
        ImGui::SliderFloat("Cam look x", &camera.lookAt.x, -10, 10);
        ImGui::SliderFloat("Cam look y", &camera.lookAt.y, -10, 10);
        ImGui::SliderFloat("Cam look z", &camera.lookAt.z, -10, 10);
        ImGui::Checkbox("DarkMode", &DarkMode);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        camera.update();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Engine::createComputeShader(std::string shaderName) {
    raytracer = ComputeShader(shaderName.c_str());
}

void Engine::createShaderProgram(std::string vertexShaderName, std::string fragmentShaderName) {
    shader = Shader(vertexShaderName.c_str(), fragmentShaderName.c_str());
}

void Engine::RenderSingleFrame(int width, int height, int rpp, int mrb, std::string outputName) {
    unsigned int raytracedTexture;

    glGenTextures(1, &raytracedTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, raytracedTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL); // Correct initialization
    glBindImageTexture(0, raytracedTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F); // Use WRITE_ONLY

    raytracer.use();
    raytracer.setInt("RayPerPixel", rpp);
    raytracer.setInt("MaxRayBounce", mrb);
    raytracer.setFloat3("ViewParams", camera.viewParams);
    raytracer.setFloat3("ViewCenter", camera.get_center());
    raytracer.setFloat44("View", camera.view);
    raytracer.dispatch(ceil(width / 16.0), ceil(height / 16.0), 1, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    float* pixels = new float[width * height * 4];
    unsigned char* pixelsData = new unsigned char[width * height * 4];

    glBindTexture(GL_TEXTURE_2D, raytracedTexture);
    // Read float data into the float buffer
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);

    // Correctly convert float data to unsigned char
    for (int i = 0; i < width * height * 4; ++i) {
        float clamped = std::clamp(pixels[i], 0.0f, 1.0f);
        pixelsData[i] = static_cast<unsigned char>(clamped * 255.0f);
    }

    std::string outName = outputName + ".png";

    stbi_write_png(outName.c_str(), width, height, 4, pixelsData, width * 4);

    delete[] pixels;
    delete[] pixelsData;
    glDeleteTextures(1, &raytracedTexture);
}




