//
// Created by Samuel on 2025-07-29.
//

#ifndef RAY_H
#define RAY_H
#include <float.h>
#include <glm/glm.hpp>
#include <memory>

#include "Material.h"

struct HitInfo {
    bool hit;
    float hitDist;
    glm::vec3 hitPosition;
    glm::vec3 normal;
    glm::vec2 texCoords;
    std::shared_ptr<Material> material;
};

class Ray {
    public:
        Ray(glm::vec3 origin, glm::vec3 direction) : orig(origin), dir(direction) {};
        glm::vec3 origin() {
            return orig;
        }
        glm::vec3 direction() {
            return dir;
        }
        glm::vec3 at(float t) {
            return orig + t * dir;
        }

        void setOrigin(glm::vec3 newOrigin) {
            orig = newOrigin;
        }
        void setDirection(glm::vec3 newDirection) {
            dir = newDirection;
        }
    private:
        glm::vec3 orig;
        glm::vec3 dir;
};



#endif //RAY_H
