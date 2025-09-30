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
           int image_width, int image_height, glm::vec3 lookAt, glm::vec3 up) : fov(fov),center(center), image_width(image_width),
                                                                                image_height(image_height), lookAt(lookAt), up(up) {
        focal_length = glm::length(center - lookAt);
        auto angleRad = fov * M_PI / 180.0f;
        viewport_height = 2.0f * std::tan(angleRad / 2) * focal_length;
        aspect_ratio = (float)image_width / (float)image_height;
        viewport_width = viewport_height * aspect_ratio;

        view = glm::inverse(glm::lookAt(center, lookAt, up));

        float planeHeight = focal_length * tan(angleRad / 2) * 2;
        float planeWidth = planeHeight * aspect_ratio;

        viewParams = glm::vec3(planeWidth, planeHeight, focal_length);

        w = glm::normalize(center - lookAt); // Camera Direction
        u = glm::normalize(glm::cross(up, w)); // Camera Right
        v = glm::normalize(glm::cross(w, u)); // Camera Up Axis

        setup_viewport();
    };

    void setup_viewport();
    int get_image_width() {
        return image_width;
    }
    int get_image_height() {
        return image_height;
    }
    glm::vec3 get_pixel00_location() {
        return pixel00_location;
    }
    glm::vec3 get_pixelDelta_u() {
        return pixelDelta_u;
    }
    glm::vec3 get_pixelDelta_v() {
        return pixelDelta_v;
    }
    glm::vec3 get_center() {
        return center;
    }
    glm::vec3 get_lookAt() {
        return lookAt;
    }
    glm::vec3 get_up() {
        return up;
    }

    bool hasChanged();
    void resetChange();

    void update();
    glm::mat4 view;
    glm::vec3 viewParams;
    glm::vec3 center;
    float fov;
    glm::vec3 lookAt;
private:
    float aspect_ratio;
    float focal_length;
    int image_width;
    int image_height;

    glm::vec3 up;

    glm::vec3 u, v, w;

    float viewport_width;
    float viewport_height;
    glm::vec3 pixelDelta_u;
    glm::vec3 pixelDelta_v;
    glm::vec3 pixel00_location;

    bool hasBeenUpdated = false;
};



#endif //CAMERA_H
