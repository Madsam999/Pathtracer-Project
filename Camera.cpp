//
// Created by Samuel on 12/5/2025.
//

#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

void Camera::initializeCamera() {
    glm::mat4 view = glm::lookAt(pos, front, worldUp);

    worldToCamera = view;
    cameraToWorld = glm::inverse(view);

    glm::mat4 proj = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 100.0f);

    projection = proj;
    inverseProjection = glm::inverse(proj);

    viewProjection = proj * view;
    prevViewProjection = viewProjection;

    isDirty = false;
}

void Camera::update() {
    prevViewProjection = viewProjection;

    // We assume that projection is constant (fov and image size are both constant)

    glm::mat4 view = glm::lookAt(pos, front + pos, camUp);

    worldToCamera = view;
    cameraToWorld = glm::inverse(view);

    viewProjection = projection * view;
}
