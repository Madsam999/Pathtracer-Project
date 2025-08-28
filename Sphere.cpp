//
// Created by Samuel on 2025-07-29.
//

#include "Sphere.h"

void Sphere::print_mat_color() const {
    glm::vec3 color = this->material.getColor();
    std::cout << "(" << color.x << "," << color.y << "," << color.z << ")" << std::endl;
}

void Sphere::intersect(Ray& ray, HitInfo& hit_info) const {
    glm::vec3 oc = center - ray.origin();
    auto a = dot(ray.direction(), ray.direction());
    auto b = -2.0f * dot(ray.direction(), oc);
    auto c = dot(oc, oc) - pow(radius, 2);
    auto discriminant = b * b - 4 * a * c;
    if (discriminant >= 0) {
        float t = float(-b - sqrt(discriminant)) / (2 * a);
        glm::vec3 normal = ray.at(t) - center;
        if (t > 0.0f && t < hit_info.hitDist) {
            hit_info.hitDist = t;
            hit_info.material = material;

            hit_info.hit = true;
            hit_info.normal = glm::normalize(normal);
        }
    }
}
