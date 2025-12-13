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

    if (cursorDisabled) {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        handleMouseMove(static_cast<float>(mouseX), static_cast<float>(mouseY));
    }
}

void CameraController::handleKeyboardInput(int key) {
    glm::vec3 direction;
    glm::vec3 camCenter = camera.getPos();

    glm::vec3 up = camera.getUp();
    glm::vec3 right = camera.getRight();
    glm::vec3 front = camera.getLookAt();
    switch(key) {
        case UP:
            direction = up;
            camCenter += direction * speed;
            break;
        case DOWN:
            direction = -up;
            camCenter += direction * speed;
            break;
        case LEFT:
            direction = -right;
            camCenter += direction * speed;
            break;
        case RIGHT:
            direction = right;
            camCenter += direction * speed;
            break;
        case FORWARD:
            direction = front;
            camCenter += direction * speed;
            break;
        case BACKWARD:
            direction = -front;
            camCenter += direction * speed;
            break;
    };

    camera.setPosition(camCenter);
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

    glm::vec3 front = direction;
    glm::vec3 right = glm::normalize(glm::cross(front, camera.getWorldUp()));
    glm::vec3 up = glm::normalize(glm::cross(right, front));

    camera.setLookAt(front);
    camera.setRight(right);
    camera.setUp(up);
}