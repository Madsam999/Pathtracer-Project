cmake_minimum_required(VERSION 3.31)
project(Pathtracer_Project)

set(CMAKE_CXX_STANDARD 20)

file(GLOB IMGUR_SOURCES "imgui/*.cpp")

add_executable(Pathtracer_Project main.cpp
        Material.cpp
        Material.h
        Ray.cpp
        Ray.h
        Scene.cpp
        Scene.h
        Camera.cpp
        Camera.h
        Utilities/RandomUtilities.cpp
        Utilities/MathUtilities.cpp
        Shader.cpp
        Shader.h
        ComputeShader.cpp
        ComputeShader.h
        Engine.cpp
        Engine.h

        # ImGui Sources
        ImGui/imgui.cpp
        ImGui/imgui_demo.cpp
        ImGui/imgui_draw.cpp
        ImGui/imgui_impl_glfw.cpp
        ImGui/imgui_impl_glfw.h
        ImGui/imgui_impl_opengl3.cpp
        ImGui/imgui_tables.cpp
        ImGui/imgui_widgets.cpp
        CameraController.cpp
        CameraController.h
        ObjectClasses/SceneObjects.cpp
        ObjectClasses/SceneObjects.h
        ObjectClasses/SphereObject.cpp
        ObjectClasses/SphereObject.h
        ObjectClasses/MeshObject.cpp
        ObjectClasses/MeshObject.h
        ObjectClasses/Mesh.cpp
        ObjectClasses/Mesh.h
        Utilities/MeshBuilder.cpp
        Utilities/MeshBuilder.h
)

find_package(glm CONFIG REQUIRED)
target_link_libraries(Pathtracer_Project PRIVATE glm::glm)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Pathtracer_Project PRIVATE glfw)
find_package(glad CONFIG REQUIRED)
target_link_libraries(Pathtracer_Project PRIVATE glad::glad)
find_package(Stb REQUIRED)
target_include_directories(Pathtracer_Project PRIVATE ${Stb_INCLUDE_DIR})
find_package(assimp CONFIG REQUIRED)
target_link_libraries(Pathtracer_Project PRIVATE assimp::assimp)

