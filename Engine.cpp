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

    createSSBO();

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

void Engine::createSSBO() {
    sphereSSBO();
    meshSSBO();
}

void Engine::sphereSSBO() {
    std::vector<SphereObject> spheres = scene.getSpheres();
    std::vector<SphereInfo> sphereInfos;

    for (int i = 0; i < spheres.size(); i++) {
        SphereObject sphere = spheres[i];
        std::shared_ptr<Material> material = sphere.getMaterial();
        SphereInfo sphereData;
        sphereData.transform = sphere.getTransform();
        sphereData.invTransorm = sphere.getInverseTransform();
        MaterialInfo matData;
        matData.color = glm::vec4(material->getColor(), 0);
        matData.emissionColor = glm::vec4(material->getEmissionColor(), 0);
        matData.emissionStrength = material->getEmissionStrength();
        matData.specular = material->getSpecular();
        sphereData.mat = matData;

        sphereInfos.push_back(sphereData);
    }

    int numSpheres = sphereInfos.size();
    size_t sphere_header_size = 16;
    size_t sphere_data_size = sizeof(SphereInfo) * sphereInfos.size();
    std::vector<char> sphere_ssbo_data(sphere_header_size + sphere_data_size);
    std::memcpy(sphere_ssbo_data.data(), &numSpheres, sizeof(int));
    std::memset(sphere_ssbo_data.data() + sizeof(int), 0, sphere_header_size - sizeof(int));
    std::memcpy(sphere_ssbo_data.data() + sphere_header_size, sphereInfos.data(), sphere_data_size);
    GLuint ssbo_spheres;
    glGenBuffers(1, &ssbo_spheres);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_spheres);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sphere_ssbo_data.size(), sphere_ssbo_data.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo_spheres); // BINDING POINT 1
}


void Engine::meshSSBO() {
    std::vector<MeshObject> meshes = scene.getMeshes();
    std::vector<TriangleInfo> triangles;
    std::vector<MeshInfo> meshInfos;
    unsigned int currentTriangleOffset = 0;

    for (int i = 0; i < meshes[0].getMesh()->getTriangles().size(); i++) {
        auto triangle = meshes[0].getMesh()->getTriangles()[i];
        std::cout << "Triangle: " << i << std::endl;
        std::cout << "Positions: " << std::endl;
        printf("(%f, %f, %f)\n", triangle.positionA.x, triangle.positionA.y, triangle.positionA.z);
        printf("(%f, %f, %f)\n", triangle.positionB.x, triangle.positionB.y, triangle.positionB.z);
        printf("(%f, %f, %f)\n", triangle.positionC.x, triangle.positionC.y, triangle.positionC.z);
        printf("Normals: \n");
        printf("(%f, %f, %f)\n", triangle.normalA.x, triangle.normalA.y, triangle.normalA.z);
        printf("(%f, %f, %f)\n", triangle.normalB.x, triangle.normalB.y, triangle.normalB.z);
        printf("(%f, %f, %f)\n", triangle.normalC.x, triangle.normalC.y, triangle.normalC.z);
    }

    for (const auto& meshObject : meshes) {
        printf("Current triangle offset: %u\n", currentTriangleOffset);
        auto mesh = meshObject.getMesh();
        MeshInfo meshInfo;
        meshInfo.transform = meshObject.getTransform();
        meshInfo.invTransform = meshObject.getInverseTransform();
        meshInfo.boundsMin = glm::vec4(mesh->getMin(), 1.f);
        meshInfo.boundsMax = glm::vec4(mesh->getMax(), 1.f);
        meshInfo.info = glm::uvec4((unsigned int)currentTriangleOffset, (unsigned int)mesh->getTriangles().size(), 0, 0);
        std::shared_ptr<Material> meshObjectMat = meshObject.getMaterial();
        MaterialInfo matData;
        matData.color = glm::vec4(meshObjectMat->getColor(), 0);
        matData.emissionColor = glm::vec4(meshObjectMat->getEmissionColor(), 0);
        matData.emissionStrength = meshObjectMat->getEmissionStrength();
        matData.specular = meshObjectMat->getSpecular();
        meshInfo.material = matData;
        meshInfos.push_back(meshInfo);
        std::vector<Triangle> meshTriangles = mesh->getTriangles();
        std:: cout << meshTriangles.size() << std::endl;
        for (const auto& triangle : meshTriangles) {
            TriangleInfo triangleData;
            std::memcpy(&triangleData, &triangle, sizeof(TriangleInfo));
            triangleData.positionA.w = 1.f;
            triangleData.positionB.w = 1.f;
            triangleData.positionC.w = 1.f;
            triangleData.normalA.w = 0.f;
            triangleData.normalB.w = 0.f;
            triangleData.normalC.w = 0.f;
            triangles.push_back(triangleData);
        }
        currentTriangleOffset +=  meshTriangles.size();
        printf("Current triangle offset: %u\n", currentTriangleOffset);
    }

    for (int i = 0; i < triangles.size(); i++) {
        auto triangle = triangles[i];
        std::cout << "Triangle: " << i << std::endl;
        std::cout << "Positions: " << std::endl;
        printf("(%f, %f, %f)\n", triangle.positionA.x, triangle.positionA.y, triangle.positionA.z);
        printf("(%f, %f, %f)\n", triangle.positionB.x, triangle.positionB.y, triangle.positionB.z);
        printf("(%f, %f, %f)\n", triangle.positionC.x, triangle.positionC.y, triangle.positionC.z);
        printf("Normals: \n");
        printf("(%f, %f, %f)\n", triangle.normalA.x, triangle.normalA.y, triangle.normalA.z);
        printf("(%f, %f, %f)\n", triangle.normalB.x, triangle.normalB.y, triangle.normalB.z);
        printf("(%f, %f, %f)\n", triangle.normalC.x, triangle.normalC.y, triangle.normalC.z);
    }

    // Triangle SSBO Buffer
    size_t triangle_header_size = 16;
    size_t triangle_data_size = sizeof(TriangleInfo) * triangles.size();
    std::vector<char> triangle_ssbo_data(triangle_header_size + triangle_data_size);
    int numTriangles = triangles.size();
    std::memcpy(triangle_ssbo_data.data(), &numTriangles, sizeof(int));
    std::memset(triangle_ssbo_data.data() + sizeof(int), 0, triangle_header_size - sizeof(int));
    std::memcpy(triangle_ssbo_data.data() + triangle_header_size, triangles.data(), triangle_data_size);
    GLuint ssbo_triangles;
    glGenBuffers(1, &ssbo_triangles);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_triangles);
    glBufferData(GL_SHADER_STORAGE_BUFFER, triangle_ssbo_data.size(), triangle_ssbo_data.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo_triangles);

    // Mesh SSBO Buffer
    size_t mesh_header_size = 16;
    size_t mesh_data_size = sizeof(MeshInfo) * meshInfos.size();
    std::vector<char> mesh_ssbo_data(mesh_header_size + mesh_data_size);
    int numMeshes = meshInfos.size();
    std::memcpy(mesh_ssbo_data.data(), &numMeshes, sizeof(int));
    std::memset(mesh_ssbo_data.data() + sizeof(int), 0, mesh_header_size - sizeof(int));
    std::memcpy(mesh_ssbo_data.data() + mesh_header_size, meshInfos.data(), mesh_data_size);
    GLuint ssbo_meshes;
    glGenBuffers(1, &ssbo_meshes);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_meshes);
    glBufferData(GL_SHADER_STORAGE_BUFFER, mesh_ssbo_data.size(), mesh_ssbo_data.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo_meshes);
}







