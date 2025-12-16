//
// Created by Samuel on 12/4/2025.
//

#include "Engine.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void Engine::createComputeShader(std::string shaderName) {
    raytracer = ComputeShader(shaderName.c_str());
}

void Engine::createShaderProgram(std::string vertexShaderName, std::string fragmentShaderName) {
    shader = Shader(vertexShaderName.c_str(), fragmentShaderName.c_str());
}

void Engine::createGLWFContext(const char *title) {
    std::cout << "Creating OpenGL context" << std::endl;
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        std::exit(EXIT_FAILURE);
    }

    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
}

void Engine::createImGuiContext() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");
}

unsigned int Engine::createRenderTarget() {
    float quadVerts[] = {
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
    return quadVAO;
}

void Engine::raytracePass(int frame, int currentFrame, int historyFrame) {
    raytracer.use();

    raytracer.setInt("frameCnt", frame);
    raytracer.setInt("RayPerPixel", rpp);
    raytracer.setInt("MaxRayBounce", mrb);
    raytracer.setFloat3("ViewCenter", camera->getPos());
    raytracer.setBool("denoiserActive", denoiserActive);

    // Matrix Uniforms
    raytracer.setFloat44("InverseProjection", camera->getInverseProjection());
    raytracer.setFloat44("CameraToWorld", camera->getInverseView());
    raytracer.setFloat44("CurrentVP", camera->getViewProjection());
    raytracer.setFloat44("PrevVP", camera->getPreviousViewProjection());

    denoiser.bindTexture(currentFrame);

    raytracer.dispatch(ceil(width / 16.f), ceil(height / 16.f), 1, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Engine::accumulationPass(int frame, int currentFrame, int historyFrame) {
    denoiser.accumulationPass(currentFrame, historyFrame, frame);
}

void Engine::varianceEstimatePass(int currentFrame) {
    denoiser.varianceEstimatePass(currentFrame);
}

void Engine::atrousFilterPass(int currentFrame, int historyFrame) {
    denoiser.atrousFilterPass(currentFrame, historyFrame);
}


void Engine::renderToScreen(int currentFrame, unsigned int quadVAO) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int currentDebugMode = 0;
    GLuint textureToDisplay = 0;

    switch (denoiserActive) {
        case false:
            textureToDisplay = denoiser.getNoisyTexture();
            break;
        case true:
            textureToDisplay = denoiser.getDenoisedTexture(currentFrame);
            break;
    }

    shader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureToDisplay);
    glUniform1i(glGetUniformLocation(shader.getProgram(), "u_DebugTexture"), 0);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Engine::renderGUI() {
    // --- 1. Settings Window ---
    ImGui::Begin("Settings");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::SliderInt("Ray Per Pixel", &rpp, 1, 1000);
    ImGui::SliderInt("Max Ray Bounce", &mrb, 1, 1000);
    ImGui::End();

    // --- 2. Camera Info Window ---
    ImGui::Begin("Camera info");
    ImGui::Text("Position");
    ImGui::Text("x: %f", camera->getPos().x);
    ImGui::Text("y: %f", camera->getPos().y);
    ImGui::Text("z: %f", camera->getPos().z);
    ImGui::End();

    bool enabled = false;
    ImGui::Begin("Denoiser Settings");
    ImGui::Text("Denoiser Settings");
    if (ImGui::Checkbox("Denoiser Active", &denoiserActive)) {
    }
    ImGui::End();

    // --- Final Render ---
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Engine::handleInputEvents() {
    cameraController->handleInputEvent(window);
}

void Engine::takeScreenShot(GLuint denoisedTexture) {
    std::string outputName = "Screenshot_" + std::to_string(screenShots) + ".png";

    std::string folderPath = "Results/Screenshots/";
    if (!fs::exists(folderPath)) {
        fs::create_directories(folderPath);
    }

    std::string fullSavePath = folderPath + outputName;

    const int channels = 4;
    size_t bufferSize = (size_t)width * height * channels;
    unsigned char* buffer = new unsigned char[bufferSize];
    glBindFramebuffer(GL_READ_FRAMEBUFFER, denoisedTexture);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    stbi_flip_vertically_on_write(1);
    int success = stbi_write_png(fullSavePath.c_str(), width, height, channels, buffer, width * channels);
    stbi_flip_vertically_on_write(0);
    if (success) {
        printf("Screenshot %s saved successfully\n", fullSavePath.c_str());
    }
    else {
        printf("Failed to save screenshot %s\n", fullSavePath.c_str());
    }
    delete[] buffer;
    screenShots++;
}



void Engine::run() {
    unsigned int quadVAO = createRenderTarget();

    int frameCount = 0;

    int currentFrame = 0;
    int historyFrame = 1;

    initializeSSBO();

    double startTime = glfwGetTime();

    glm::vec3 initPos = scene->getSpheres()[0].getPosition();

    while (!glfwWindowShouldClose(window)) {

        handleInputEvents();
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
            takeScreenShot(denoiser.getDenoisedTexture(currentFrame));
        }

        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        frameCount++;

        raytracePass(frameCount, currentFrame, historyFrame);

        if (denoiserActive) {
            accumulationPass(frameCount, currentFrame, historyFrame);

            varianceEstimatePass(currentFrame);
            atrousFilterPass(currentFrame, historyFrame);
        }

        renderToScreen(currentFrame, quadVAO);

        renderGUI();

        glfwSwapBuffers(window);
        glfwPollEvents();

        std::swap(currentFrame, historyFrame);
    }
}

void Engine::initializeSSBO() {
    initializeSphereSSBO();
    initializeMeshSSBO();
}

void Engine::initializeSphereSSBO() {
    std::vector<SphereObject> spheres = scene->getSpheres();
    std::vector<SphereInfo> sphereInfos;

    for (int i = 0; i < spheres.size(); i++) {
        SphereObject sphere = spheres[i];
        std::shared_ptr<Material> material = sphere.getMaterial();
        SphereInfo sphereData;
        sphereData.transform = sphere.getTransform();
        sphereData.invTransorm = sphere.getInverseTransform();
        sphereData.prevTransform = sphere.getPrevTransform();
        sphereData.prevInverseTransform = sphere.getPrevInverseTransform();
        MaterialInfo matData;
        matData.color = glm::vec4(material->getColor(), 0);
        matData.emissionColor = glm::vec4(material->getEmissionColor(), 0);
        matData.emissionStrength = material->getEmissionStrength();
        matData.specular = material->getSpecular();
        sphereData.mat = matData;
        sphereData.objectID = glm::uvec4(sphere.getObjectID(), 0, 0, 1);

        sphereInfos.push_back(sphereData);
    }

    int numSpheres = sphereInfos.size();
    size_t sphere_header_size = 16;
    size_t sphere_data_size = sizeof(SphereInfo) * sphereInfos.size();
    std::vector<char> sphere_ssbo_data(sphere_header_size + sphere_data_size);
    std::memcpy(sphere_ssbo_data.data(), &numSpheres, sizeof(int));
    std::memset(sphere_ssbo_data.data() + sizeof(int), 0, sphere_header_size - sizeof(int));
    std::memcpy(sphere_ssbo_data.data() + sphere_header_size, sphereInfos.data(), sphere_data_size);
    glGenBuffers(1, &sphereSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sphere_ssbo_data.size(), sphere_ssbo_data.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sphereSSBO); // BINDING POINT 1
}

void Engine::initializeMeshSSBO() {
    std::vector<MeshObject> meshes = scene->getMeshes();
    std::vector<TriangleInfo> triangles;
    std::vector<MeshInfo> meshInfos;
    unsigned int currentTriangleOffset = 0;

    for (auto meshObject : meshes) {
        auto mesh = meshObject.getMesh();
        MeshInfo meshInfo;
        meshInfo.transform = meshObject.getTransform();
        meshInfo.invTransform = meshObject.getInverseTransform();
        meshInfo.prevTransform = meshObject.getPrevTransform();
        meshInfo.prevInverseTransform = meshObject.getInverseTransform();
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
        meshInfo.objectID = glm::uvec4(meshObject.getObjectID(), 0, 0, 0);
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
    }

    // Triangle SSBO Buffer
    size_t triangle_header_size = 16;
    size_t triangle_data_size = sizeof(TriangleInfo) * triangles.size();
    std::vector<char> triangle_ssbo_data(triangle_header_size + triangle_data_size);
    int numTriangles = triangles.size();
    std::memcpy(triangle_ssbo_data.data(), &numTriangles, sizeof(int));
    std::memset(triangle_ssbo_data.data() + sizeof(int), 0, triangle_header_size - sizeof(int));
    std::memcpy(triangle_ssbo_data.data() + triangle_header_size, triangles.data(), triangle_data_size);
    glGenBuffers(1, &triangleSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangleSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, triangle_ssbo_data.size(), triangle_ssbo_data.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, triangleSSBO);

    // Mesh SSBO Buffer
    size_t mesh_header_size = 16;
    size_t mesh_data_size = sizeof(MeshInfo) * meshInfos.size();
    std::vector<char> mesh_ssbo_data(mesh_header_size + mesh_data_size);
    int numMeshes = meshInfos.size();
    std::memcpy(mesh_ssbo_data.data(), &numMeshes, sizeof(int));
    std::memset(mesh_ssbo_data.data() + sizeof(int), 0, mesh_header_size - sizeof(int));
    std::memcpy(mesh_ssbo_data.data() + mesh_header_size, meshInfos.data(), mesh_data_size);
    glGenBuffers(1, &meshSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, mesh_ssbo_data.size(), mesh_ssbo_data.data(), GL_STATIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, meshSSBO);
}

// Temporary function
void Engine::updateMovingSphere(int sphereIndex) {

    SphereObject& sphere = scene->getSpheres()[sphereIndex];
    glm::mat4 transform = sphere.getTransform();
    glm::mat4 invTransform = glm::inverse(transform);
    glm::mat4 prevTransform = sphere.getPrevTransform();
    glm::mat4 prevInvTransform = sphere.getPrevInverseTransform();

    // SSBO Metadata (must match your createSSBO() logic)
    const size_t sphereHeaderSize = 16;
    const size_t sphereInfoSize = sizeof(SphereInfo);

    // Offset to the start of the specific SphereInfo struct
    size_t startOffset = sphereHeaderSize + (sphereIndex * sphereInfoSize);

    // The SphereInfo struct starts with the transform matrix (mat4)
    size_t dataOffset = startOffset + offsetof(SphereInfo, transform);

    // Size of the data chunk to update (transform + inverse transform)
    size_t updateSize = 4 * sizeof(glm::mat4);

    // 1. Create a contiguous data block for the two matrices
    std::vector<char> updateData(updateSize);
    std::memcpy(updateData.data(), glm::value_ptr(transform), sizeof(glm::mat4));
    std::memcpy(updateData.data() + sizeof(glm::mat4), glm::value_ptr(invTransform), sizeof(glm::mat4));
    std::memcpy(updateData.data() + 2 * sizeof(glm::mat4), glm::value_ptr(prevTransform), sizeof(glm::mat4));
    std::memcpy(updateData.data() + 3 * sizeof(glm::mat4), glm::value_ptr(prevInvTransform), sizeof(glm::mat4));

    // 2. Bind the SSBO
    // Assuming you have the SSBO handle stored (e.g., ssbo_spheres_handle)
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphereSSBO);

    // 3. Perform the sub-data update
    glBufferSubData(
        GL_SHADER_STORAGE_BUFFER,
        dataOffset,              // Start writing at the transform matrix
        updateSize,              // Write the transform and inverse transform
        updateData.data()
    );

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind
}
