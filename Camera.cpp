//
// Created by Samuel on 2025-07-30.
//

#include "Camera.h"

void Camera::update() {
    glm::mat4 OldCameraToWorld = CameraToWorld;
    glm::mat4 OldViewProjection = ViewProjection;

    WorldToCamera = glm::lookAt(center, center + front, up);
    CameraToWorld = glm::inverse(WorldToCamera);

    PreviousCameraToWorld = OldCameraToWorld;

    ViewProjection = CameraToWorld * Projection;

    PreviousViewProjection = OldViewProjection;
}