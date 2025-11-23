//
// Created by Samuel on 11/12/2025.
//

#ifndef SPHEREOBJECT_H
#define SPHEREOBJECT_H
#include <utility>

#include "SceneObjects.h"
#include "../Material.h"


class SphereObject : public SceneObject {
public:

    SphereObject(
        float radius,
        glm::vec3 position,
        glm::vec3 rotation,
        glm::vec3 scale,
        std::shared_ptr<Material> material) :
        SceneObject(
            position,
            rotation,
            scale,
            std::move(material)
        ),
        radius(radius) {
        printf("Sphere with ID %llu created!", this->getObjectID());
    }

    [[nodiscard]] float getRadius() const {
        return radius;
    }

    void localIntersect(Ray &ray, HitInfo &hit_info) const override;

private:
    float radius;
};



#endif //SPHEREOBJECT_H
