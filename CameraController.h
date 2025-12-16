//
// Created by Samuel on 11/1/2025.
//

#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include "Camera.h"

#include <GLFW/glfw3.h>

#include "Camera.h"

enum {
    UP = GLFW_KEY_SPACE,
    DOWN = GLFW_KEY_CAPS_LOCK,
    LEFT = GLFW_KEY_A,
    RIGHT = GLFW_KEY_D,
    FORWARD = GLFW_KEY_W,
    BACKWARD = GLFW_KEY_S,
};

class CameraController {
public:
    CameraController(Camera& camera, float speed, float sensitivity) : camera(camera), speed(speed), sensitivity(sensitivity) {}

    void handleInputEvent(GLFWwindow* window);
private:
    void handleKeyboardInput(int key);
    void handleMouseButtonInput();
    void handleMouseScroll();
    void handleMouseMove(float mouseX, float mouseY);
    Camera& camera;
    float speed;
    float sensitivity;

    float lastX, lastY;
    bool firstMouse = true;
    float yaw, pitch = 0.0f;

    bool cursorDisabled = true;
    double mouseX, mouseY;
};



#endif //CAMERACONTROLLER_H
