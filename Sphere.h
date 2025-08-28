//
// Created by Samuel on 2025-07-29.
//

#ifndef SPHERE_H
#define SPHERE_H
#include <iostream>
#include <ostream>

#include "Material.h"
#include "Object.h"

class Sphere : public Object {
    public:
        Sphere(Material material, glm::vec3 center, float radius) : Object(material), center(center), radius(radius) {
            std::cout << "Built" << std::endl;
        }
        void print_mat_color() const override;
        void intersect(Ray& ray, HitInfo& hit_info) const override;
        glm::vec3 get_center() const { return center; }
        float get_radius() const { return radius; }
        Material get_material() const { return material; }
    private:
        glm::vec3 center;
        float radius;
};



#endif //SPHERE_H
