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
        WorldToCamera = glm::lookAt(center, lookAt, worldUp);
        CameraToWorld = glm::inverse(WorldToCamera);

        Projection = glm::perspective(glm::radians(fov), static_cast<float>(image_width) / static_cast<float>(image_height), 0.1f, 100.0f);
        InverseProjection = glm::inverse(Projection);

        ViewProjection = Projection * CameraToWorld;
        PreviousViewProjection = ViewProjection;

        PreviousCameraToWorld = CameraToWorld;
    };

    int get_image_width() {
        return image_width;
    }
    int get_image_height() {
        return image_height;
    }

    [[nodiscard]] glm::mat4 camera_to_world() const {
        return CameraToWorld;
    }

    [[nodiscard]] glm::mat4 world_to_camera() const {
        return WorldToCamera;
    }

    [[nodiscard]] glm::mat4 projection() const {
        return Projection;
    }

    [[nodiscard]] glm::mat4 inverse_projection() const {
        return InverseProjection;
    }

    [[nodiscard]] glm::mat4 view_projection() const {
        return ViewProjection;
    }

    [[nodiscard]] glm::mat4 previous_view_projection() const {
        return PreviousViewProjection;
    }

    [[nodiscard]] glm::mat4 previous_camera_to_world() const {
        return PreviousCameraToWorld;
    }

    [[nodiscard]] glm::vec3 center1() const {
        return center;
    }

    [[nodiscard]] glm::vec3 front1() const {
        return front;
    }

    [[nodiscard]] glm::vec3 right1() const {
        return right;
    }

    [[nodiscard]] glm::vec3 up1() const {
        return up;
    }

    [[nodiscard]] glm::vec3 world_up() const {
        return worldUp;
    }

    void resetFlag() {
        updateFlag = false;
    }

    bool checkFlag() {
        return updateFlag;
    }

    void set_front(const glm::vec3 &front) {
        this->front = front;
    }

    void set_right(const glm::vec3 &right) {
        this->right = right;
    }

    void set_up(const glm::vec3 &up) {
        this->up = up;
    }

    void set_center(const glm::vec3 &center) {
        this->center = center;
    }

    void needsUpdate() {
        updateFlag = true;
    }

    void update();
    float fov;
private:
    float aspect_ratio;
    float focal_length;
    int image_width;
    int image_height;

    glm::mat4 CameraToWorld; // View_t^-1
    glm::mat4 WorldToCamera; // View_t

    glm::mat4 Projection;
    glm::mat4 InverseProjection;

    glm::mat4 ViewProjection; // VP_t = P * View_t
    glm::mat4 PreviousViewProjection; //VP_{t-1} = P * View_{t-1}

    glm::mat4 PreviousCameraToWorld; // View_{t-1}

    glm::vec3 center;
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
    glm::vec3 worldUp;

    bool updateFlag;
};



#endif //CAMERA_H
