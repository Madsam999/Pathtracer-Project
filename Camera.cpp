//
// Created by Samuel on 2025-07-30.
//

#include "Camera.h"

void Camera::update() {
    glm::mat4 newView = glm::inverse(glm::lookAt(center, center + front, up));
    auto angleRad = fov * M_PI / 180.0f;
    float planeHeight = focal_length * tan(angleRad / 2) * 2;
    float planeWidth = planeHeight * aspect_ratio;

    glm::vec3 newViewParams = glm::vec3(planeWidth, planeHeight, focal_length);

    if (!glm::all(glm::equal(newView, view, 0.0001f)) || !glm::all(glm::equal(newViewParams, viewParams, 0.0001f))) {
        hasBeenUpdated = true;
    }

    view = newView;
    viewParams = newViewParams;
}

bool Camera::hasChanged() {
    return hasBeenUpdated;
}

void Camera::resetChange() {
    hasBeenUpdated = false;
}