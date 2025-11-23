//
// Created by Samuel on 2025-07-30.
//
#pragma once

#ifndef CAMERA_H
#define CAMERA_H
#include <glm/vec3.hpp>

#include <cmath>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_relational.hpp>

class Camera {
public:
    Camera() {}

    Camera(float fov, glm::vec3 center,
           int image_width, int image_height, glm::vec3 lookAt, glm::vec3 worldUp) : fov(fov),center(center), image_width(image_width),
                                                                                image_height(image_height), worldUp(worldUp) {
        focal_length = glm::length(center - lookAt);
        auto angleRad = fov * M_PI / 180.0f;
        aspect_ratio = (float)image_width / (float)image_height;

        view = glm::inverse(glm::lookAt(center, lookAt, worldUp));

        float planeHeight = focal_length * tan(angleRad / 2) * 2;
        float planeWidth = planeHeight * aspect_ratio;

        viewParams = glm::vec3(planeWidth, planeHeight, focal_length);

        front = glm::normalize(center - lookAt); // Camera Direction
        right = glm::normalize(glm::cross(front, worldUp)); // Camera Right
        up = glm::normalize(glm::cross(right, front)); // Camera Up Axis
    };

    void setup_viewport();
    int get_image_width() {
        return image_width;
    }
    int get_image_height() {
        return image_height;
    }

    bool hasChanged();
    void resetChange();

    void update();
    glm::mat4 view;
    glm::vec3 viewParams;
    glm::vec3 center;
    float fov;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 worldUp;
private:
    float aspect_ratio;
    float focal_length;
    int image_width;
    int image_height;

    bool hasBeenUpdated = false;
};



#endif //CAMERA_H
