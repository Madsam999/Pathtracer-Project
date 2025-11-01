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

void resetAccumulation(unsigned int textureID) {
    const float zero[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearTexImage(textureID, 0, GL_RGBA, GL_FLOAT, &zero);
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
    std::cout << "Creating window: " << name << std::endl;
    std::cout << "Width: " << width << std::endl;
    std::cout << "Height: " << height << std::endl;
    if (!glfwInit()) {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLFW3");
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);

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

    CameraController camController(camera, 0.05, 0.1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");
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

    unsigned int accumulationTexture, raytracedTexture;

    // Setup for accumulation texture
    glGenTextures(1, &accumulationTexture);
    glBindTexture(GL_TEXTURE_2D, accumulationTexture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Allocate memory block for texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, accumulationTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    resetAccumulation(accumulationTexture);

    // Setup for raytraced texture
    glGenTextures(1, &raytracedTexture);
    glBindTexture(GL_TEXTURE_2D, raytracedTexture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Allocate memory block for texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glBindImageTexture(1, raytracedTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Activate texture for fragment shader sampler2D
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, raytracedTexture);
    shader.setInt("tex", 0);

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
    bool sunset = false;

    int frame = 0;
    bool resetNeeded = false;

    int screenShots = 0;
    std::string outputName = "Single_Frame";

    while (!glfwWindowShouldClose(window)) {

        if (camera.hasChanged() || resetNeeded) {
            frame = 0;
            resetAccumulation(accumulationTexture);
            camera.resetChange();
            resetNeeded = false;
        }

        camController.handleInputEvent(window);
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            RenderSingleFrame(outputName + std::to_string(screenShots) + ".png", raytracedTexture);
            screenShots++;
            printf("Rendered single frame");
        }


        frame++;

        raytracer.use();

        raytracer.setInt("frameCnt", frame);
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
        if (ImGui::SliderInt("RPP", &rpp, 1, 1000)) {resetNeeded = true;}
        if (ImGui::SliderInt("Mrb", &mrb, 1, 1000)) {resetNeeded = true;}
        ImGui::ColorEdit3("color", glm::value_ptr(RedSphereColor));
        ImGui::ColorEdit3("light", glm::value_ptr(LightColor));
        if (ImGui::Checkbox("DarkMode", &DarkMode)) {resetNeeded = true;}
        ImGui::End();

        ImGui::Begin("Camera info");
        ImGui::Text("Position");
        ImGui::Text("x: %f", camera.center.x);
        ImGui::Text("y: %f", camera.center.y);
        ImGui::Text("z: %f", camera.center.z);
        ImGui::Text("Front");
        ImGui::Text("x: %f", camera.front.x);
        ImGui::Text("y: %f", camera.front.y);
        ImGui::Text("z: %f", camera.front.z);
        ImGui::Text("Up");
        ImGui::Text("x: %f", camera.up.x);
        ImGui::Text("y: %f", camera.up.y);
        ImGui::Text("z: %f", camera.up.z);
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

// --- CORRECTED RenderSingleFrame FUNCTION ---
void Engine::RenderSingleFrame(std::string outputName, GLuint texture) {
    // 1. Prepare buffer for 8-bit unsigned char data (for stbi_write_png)
    // We are reading the final LDR image from the default framebuffer (the screen).
    const int channels = 4;
    size_t bufferSize = (size_t)width * height * channels;
    unsigned char* pixelData = new unsigned char[bufferSize];

    // 2. Read pixels from the default framebuffer (the screen)
    // The screen (window) framebuffer is always framebuffer 0.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // glReadPixels reads the color data (which is LDR: 0-255)
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);

    // 3. PNG Writing setup
    // OpenGL's (0,0) is bottom-left, while image file formats are top-left.
    // We tell stbi_write_png to flip the image vertically to correct the orientation.
    stbi_flip_vertically_on_write(1);

    // 4. Write the PNG
    int success = stbi_write_png(outputName.c_str(), width, height, channels, pixelData, width * channels);

    // 5. Reset flip state
    stbi_flip_vertically_on_write(0);

    if (success) {
        printf("Screenshot taken successfully. Result in %s\n", outputName.c_str());
    }
    else {
        printf("Failed to write screenshot.\n");
    }

    // 6. Clean up
    delete[] pixelData;
}





