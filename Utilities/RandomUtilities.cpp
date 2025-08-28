//
// Created by Samuel on 2025-07-31.
//

#include <random>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>

inline float randomFloat() {
    static std::uniform_real_distribution<float> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline float randomFloat(float min, float max) {
    return min + (max - min) * randomFloat();
}

static glm::vec3 randomVec3() {
    return glm::vec3(randomFloat(), randomFloat(), randomFloat());
}

static glm::vec3 randomVec3(float min, float max) {
    return glm::vec3(randomFloat(min, max), randomFloat(), randomFloat());
}

inline glm::vec3 random_unit_vector() {
    while (true) {
        auto p = randomVec3(-1.0f, 1.0f);
        auto lensq = glm::length(p) * glm::length(p);
        if (1e-160 < lensq && lensq <= 1.0f) {
            return glm::normalize(p);
        }
    }
}

inline glm::vec3 onUnitSphere(glm::vec3 normal) {
    glm::vec3 p = random_unit_vector();
    if (glm::dot(p, normal) > 0.0f) {
        return normal;
    }
    return -normal;
}


