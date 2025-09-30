//
// Created by Samuel on 2025-07-30.
//

#include "Camera.h"

void Camera::setup_viewport() {
    glm::vec3 viewport_u = u * viewport_width;
    glm::vec3 viewport_v = -v * viewport_height;

    pixelDelta_u = viewport_u / float(image_width);
    pixelDelta_v = viewport_v / float(image_height);

    glm::vec3 viewport_upperLeft = center - glm::vec3(0.0f, 0.0f, focal_length)
                                          - viewport_u / 2.0f
                                          - viewport_v / 2.0f;

    pixel00_location = center - (focal_length * w) - viewport_u / 2.0f - viewport_v / 2.0f;
}

void Camera::update() {
    glm::mat4 newView = glm::inverse(glm::lookAt(center, lookAt, up));
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