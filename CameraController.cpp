//
// Created by Samuel on 11/1/2025.
//

#include "CameraController.h"

#include <ranges>

void CameraController::handleInputEvent(GLFWwindow *window) {
    if(glfwGetKey(window, UP) == GLFW_PRESS) {
        handleKeyboardInput(UP);
    }
    if(glfwGetKey(window, DOWN) == GLFW_PRESS) {
        handleKeyboardInput(DOWN);
    }
    if(glfwGetKey(window, LEFT) == GLFW_PRESS) {
        handleKeyboardInput(LEFT);
    }
    if(glfwGetKey(window, RIGHT) == GLFW_PRESS) {
        handleKeyboardInput(RIGHT);
    }
    if(glfwGetKey(window, FORWARD) == GLFW_PRESS) {
        handleKeyboardInput(FORWARD);
    }
    if(glfwGetKey(window, BACKWARD) == GLFW_PRESS) {
        handleKeyboardInput(BACKWARD);
    }
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if(cursorDisabled) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            cursorDisabled = false;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            cursorDisabled = true;
        }
    }

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    handleMouseMove(static_cast<float>(mouseX), static_cast<float>(mouseY));
}

void CameraController::handleKeyboardInput(int key) {
    glm::vec3 direction;
    switch(key) {
        case UP:
            direction = camera.up;
            camera.center += direction * speed;
            break;
        case DOWN:
            direction = -camera.up;
            camera.center += direction * speed;
            break;
        case LEFT:
            direction = -camera.right;
            camera.center += direction * speed;
            break;
        case RIGHT:
            direction = camera.right;
            camera.center += direction * speed;
            break;
        case FORWARD:
            direction = camera.front;
            camera.center += direction * speed;
            break;
        case BACKWARD:
            direction = -camera.front;
            camera.center += direction * speed;
            break;
    };
    camera.update();
}

void CameraController::handleMouseMove(float mouseX, float mouseY) {
    if (firstMouse) {
        firstMouse = false;
        lastX = mouseX;
        lastY = mouseY;
    }
    float offsetX = mouseX - lastX;
    float offsetY = lastY - mouseY;
    lastX = mouseX;
    lastY = mouseY;

    offsetX *= sensitivity;
    offsetY *= sensitivity;

    yaw += offsetX;
    pitch += offsetY;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    camera.front = direction;
    camera.right = glm::normalize(glm::cross(camera.front, camera.worldUp));
    camera.up = glm::normalize(glm::cross(camera.right, camera.front));

    camera.update();
}