//
// Created by Samuel on 11/12/2025.
//

#ifndef SCENEOBJECT_H
#define SCENEOBJECT_H
#include <memory>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "../Material.h"
#include "../Ray.h"


class SceneObject {
public:
    virtual ~SceneObject() = default;

    [[nodiscard]] uint64_t getObjectID() const {
        return objectID;
    }

    [[nodiscard]] glm::vec3 getPosition() const {
        return position;
    }
    [[nodiscard]] glm::vec3 getScale() const {
        return scale;
    }
    [[nodiscard]] glm::vec3 getRotation() const {
        return rotation;
    }

    [[nodiscard]] glm::mat4 getTransform() {
        if (isDirty) {
            updateTransform();
            isDirty = false;
        }
        return transform;
    }
    [[nodiscard]] glm::mat4 getInverseTransform() {
        if (isDirty) {
            updateTransform();
            isDirty = false;
        }
        return inverseTransform;
    }
    [[nodiscard]] glm::mat3 getNormalTransform() const {
        if (isDirty) {
            updateTransform();
            isDirty = false;
        }
        return normalTransform;
    }
    [[nodiscard]] glm::mat4 getPrevTransform() const {
        if (isDirty) {
            updateTransform();
            isDirty = false;
        }
        return prevTransform;
    }
    [[nodiscard]] glm::mat4 getPrevInverseTransform() const {
        if (isDirty) {
            updateTransform();
            isDirty = false;
        }
        return prevInverseTransform;
    }

    [[nodiscard]] std::shared_ptr<Material> getMaterial() const {
        return material;
    }

    void setPosition(const glm::vec3 position) {
        this->position = position;
        isDirty = true;
    }
    void setScale(const glm::vec3 scale) {
        this->scale = scale;
        isDirty = true;
    }
    void setRotation(const glm::vec3 rotation) {
        this->rotation = rotation;
        isDirty = true;
    }

    void intersect(Ray& ray, HitInfo& hit_info);
    virtual void localIntersect(Ray& ray, HitInfo& hit_info) const = 0;
private:
    static uint64_t nextID;
    const uint64_t objectID;

    std::shared_ptr<Material> material;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    mutable glm::mat4 transform;
    mutable glm::mat4 inverseTransform;
    mutable glm::mat3 normalTransform;
    mutable glm::mat4 prevTransform;
    mutable glm::mat4 prevInverseTransform;
    mutable bool isDirty = false;

    void updateTransform() const;

    static glm::mat4 createTransform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);

    void buildNormalTransform() const;

protected:

    SceneObject(
        glm::mat4 transform,
        std::shared_ptr<Material> material,
        glm::vec3 position,
        glm::vec3 rotation,
        glm::vec3 scale) :
        transform(transform),
        material(std::move(material)),
        objectID(nextID++),
        position(position),
        rotation(rotation),
        scale(scale),
        inverseTransform(glm::inverse(transform)),
        prevTransform(transform),
        prevInverseTransform(glm::inverse(transform))
    {
        // Empty Constructor
    }

    SceneObject(
                glm::vec3 position,
                glm::vec3 rotation,
                glm::vec3 scale,
                std::shared_ptr<Material> material
                ) :
                SceneObject(
                    createTransform(position, rotation, scale),
                    std::move(material),
                    position,
                    rotation,
                    scale
                ) {
        // Empty Constructor
    }

};



#endif //SCENEOBJECT_H

