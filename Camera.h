//
// Created by Samuel on 12/5/2025.
//

#ifndef CAMERA_CLEAN_H
#define CAMERA_CLEAN_H
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/ext/matrix_transform.hpp>


class Camera {
public:
    Camera(
        float fov,
        glm::vec3 pos,
        glm::vec3 lookAt,
        glm::vec3 worldUp,
        float aspectRatio
    ) {
        this->pos = pos;
        this->front = lookAt;
        this->camUp = worldUp;
        this->worldUp = worldUp;
        this->aspectRatio = aspectRatio;
        this->fov = fov;

        initializeCamera();
    }

    [[nodiscard]] glm::mat4 getProjection() {
        if (isDirty) {
            update();
            isDirty = false;
        }
        return projection;
    }
    [[nodiscard]] glm::mat4 getInverseProjection() {
        if (isDirty) {
            update();
            isDirty = false;
        }
        return inverseProjection;
    }
    [[nodiscard]] glm::mat4 getView() {
        if (isDirty) {
            update();
            isDirty = false;
        }
        return worldToCamera;
    }
    [[nodiscard]] glm::mat4 getInverseView() {
        if (isDirty) {
            update();
            isDirty = false;
        }
        return cameraToWorld;
    }
    [[nodiscard]] glm::mat4 getViewProjection() {
        if (isDirty) {
            update();
            isDirty = false;
        }
        return viewProjection;
    }
    [[nodiscard]] glm::mat4 getPreviousViewProjection() {
        if (isDirty) {
            update();
            isDirty = false;
        }
        return prevViewProjection;
    }

    void setPosition(glm::vec3 pos) {
        this->pos = pos;
        isDirty = true;
    }
    void setLookAt(glm::vec3 lookAt) {
        this->front = lookAt;
        isDirty = true;
    }
    void setUp(glm::vec3 up) {
        this->camUp = up;
        isDirty = true;
    }
    void setRight(glm::vec3 right) {
        this->right = right;
        isDirty = true;
    }

    [[nodiscard]] glm::vec3 getPos() {
        return pos;
    }
    [[nodiscard]] glm::vec3 getLookAt() {
        return front;
    }
    [[nodiscard]] glm::vec3 getUp() {
        return camUp;
    }
    [[nodiscard]] glm::vec3 getRight() {
        return right;
    }
    [[nodiscard]] glm::vec3 getWorldUp() {
        return worldUp;
    }

private:
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 worldUp;
    glm::vec3 camUp;
    glm::vec3 right;

    glm::mat4 projection;
    glm::mat4 inverseProjection;

    glm::mat4 worldToCamera;
    glm::mat4 cameraToWorld;

    glm::mat4 viewProjection;
    glm::mat4 prevViewProjection;

    float aspectRatio;
    float fov;

    bool isDirty;

    void initializeCamera();
    void update();
};



#endif //CAMERA_CLEAN_H
