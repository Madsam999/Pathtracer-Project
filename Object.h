//
// Created by Samuel on 2025-07-29.
//

#ifndef OBJECT_H
#define OBJECT_H

#include <iostream>
#include <ostream>

#include "Material.h"
#include "Ray.h"
#include "string"
#include "glm/glm.hpp"


class Object {
    public:
        Object(Material material) : material(material){
        }
        virtual ~Object() = default;
        virtual void print_mat_color() const = 0;
        virtual void intersect(Ray& ray, HitInfo& hit_info) const = 0;
        Material get_material() const {
            return material;
        }
    protected:
        glm::vec3 center;
        Material material;
};



#endif //OBJECT_H
