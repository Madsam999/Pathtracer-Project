cmake_minimum_required(VERSION 3.31)
project(Pathtracer_Project)

set(CMAKE_CXX_STANDARD 20)

file(GLOB IMGUR_SOURCES "imgui/*.cpp")

add_executable(Pathtracer_Project main.cpp
        Object.cpp
        Object.h
        Sphere.cpp
        Sphere.h
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
        ${IMGUR_SOURCES}
)

find_package(glm CONFIG REQUIRED)
target_link_libraries(Pathtracer_Project PRIVATE glm::glm)
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Pathtracer_Project PRIVATE glfw)
find_package(glad CONFIG REQUIRED)
target_link_libraries(Pathtracer_Project PRIVATE glad::glad)
find_package(Stb REQUIRED)
target_include_directories(Pathtracer_Project PRIVATE ${Stb_INCLUDE_DIR})

