//
// Created by Samuel on 11/12/2025.
//

#include "SphereObject.h"

void SphereObject::localIntersect(Ray &ray, HitInfo &hit_info) const {
    auto a = dot(ray.direction(), ray.direction());
    auto b = 2.0f * dot(ray.direction(), ray.origin());
    auto c = dot(ray.origin(), ray.origin()) - pow(radius, 2);
    auto discriminant = b * b - 4 * a * c;
    if (discriminant >= 0) {
        float t = float(-b - sqrt(discriminant)) / (2 * a);
        glm::vec3 normal = ray.at(t) - getPosition();
        if (t > 0.0f && t < hit_info.hitDist) {
            hit_info.hitDist = t;
            hit_info.material = getMaterial();
            hit_info.hit = true;
            hit_info.normal = glm::normalize(normal);
        }
    }
}
