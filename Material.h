//
// Created by Samuel on 2025-07-29.
//

#ifndef MATERIAL_H
#define MATERIAL_H
#include <string>

#include "glm/vec3.hpp"


class Material {
    public:
        Material() : color(glm::vec3(0.0f, 0.0f, 0.0f)){}
        Material(const glm::vec3 color, glm::vec3 emissionColor, float emissionStrength, float specular) : color(color), emissionColor(emissionColor), emissionStrength(emissionStrength), specular(specular) {}
        glm::vec3 getColor() const { return color; }
        glm::vec3 getEmissionColor() const { return emissionColor; }
        float getEmissionStrength() const { return emissionStrength; }
        float getSpecular() const { return specular; }
    private:
        glm::vec3 color;
        glm::vec3 emissionColor;
        float emissionStrength;
        float specular;
};



#endif //MATERIAL_H
