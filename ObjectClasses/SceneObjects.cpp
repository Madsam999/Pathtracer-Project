//
// Created by Samuel on 11/12/2025.
//

#include "SceneObjects.h"

uint64_t SceneObject::nextID = 0;

glm::mat4 SceneObject::createTransform(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale) {
    auto transform = glm::mat4(1.0f);

    // 1. TRANSLATE (APPLIED LAST)
    // The translation matrix is multiplied last in the sequence: T * R * S
    transform = glm::translate(transform, position); // <-- MOVED TO THE TOP

    // 2. ROTATE
    // The rotations are applied relative to the object's local space
    transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0, 0, 1));

    // 3. SCALE (APPLIED FIRST)
    // Scaling is applied first in local space
    transform = glm::scale(transform, scale);

    return transform;
}

void SceneObject::updateTransform() const {
    prevTransform = transform;
    prevInverseTransform = inverseTransform;
    transform = glm::mat4(1.0f);
    transform = glm::scale(transform, scale);
    transform = glm::rotate(transform, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    transform = glm::rotate(transform, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    transform = glm::translate(transform, position);
    inverseTransform = glm::inverse(transform);
    buildNormalTransform();
}

void SceneObject::buildNormalTransform() const{
    normalTransform = glm::transpose(glm::mat3(inverseTransform));
}


void SceneObject::intersect(Ray &ray, HitInfo &hit_info) {
    glm::vec3 localOrigin = getInverseTransform() * glm::vec4(ray.origin(), 1.0f);
    glm::vec3 localDirection = getInverseTransform() * glm::vec4(ray.direction(), 0.0f);

    Ray localRay(localOrigin, localDirection);

    HitInfo localHitInfo;
    localHitInfo.hit = false;
    localHitInfo.hitDist = std::numeric_limits<float>::max();

    localIntersect(localRay, localHitInfo);
    if (localHitInfo.hit && localHitInfo.hitDist < hit_info.hitDist) {
        hit_info.hitDist = localHitInfo.hitDist;
        hit_info.material = localHitInfo.material;
        hit_info.normal = normalTransform * localHitInfo.normal;
        hit_info.hit = localHitInfo.hit;
    }
}



