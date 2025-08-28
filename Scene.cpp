//
// Created by Samuel on 2025-07-30.
//

#include "Scene.h"
#include <random>

#include <glm/gtc/matrix_transform.hpp>

glm::vec3 colorPixel(Ray ray) {
    glm::vec3 unitDir = glm::normalize(ray.direction());
    auto a = 0.5f * (unitDir.y + 1.0f);
    return (1.0f - a) * glm::vec3(1.0f, 1.0f, 1.0f) + a * glm::vec3(0.5f, 0.7f, 1.0f);
}

glm::vec3 generateRandomOffset() {
    glm::vec3 offset;
    float x = (float) std::rand() / (float) (RAND_MAX + 1.0f);
    float y = (float) std::rand() / (float) (RAND_MAX + 1.0f);
    offset = glm::vec3(x - 0.5f, y - 0.5f, 0);
    return offset;
}

std::vector<glm::vec3> Scene::render() const {
    std::vector<glm::vec3> result;
    int width = camera->get_image_width();
    int height = camera->get_image_height();
    for (int y = 0; y < height; y++) {
        std::cout << "Scanline: " << y << std::endl;
        for (int x = 0; x < width; x++) {
            glm::vec3 avgColor(0, 0, 0);
            for (int rpp = 0; rpp < ray_per_pixel; rpp++) {
                // Get ray orig and dir
                glm::vec3 offset = generateRandomOffset();
                auto pixel_center = camera->get_pixel00_location()
                                             + ((float(x) + offset.x) * camera->get_pixelDelta_u())
                                             + ((float(y) + offset.y) * camera->get_pixelDelta_v());
                auto rayDir = pixel_center - camera->get_center();
                rayDir = glm::normalize(rayDir);
                Ray ray(camera->get_center(), rayDir);

                avgColor += trace(ray);
            }
            avgColor /= ray_per_pixel;
            avgColor = glm::clamp(avgColor, 0.0f, 1.0f);

            result.push_back(avgColor);
        }
    }
    return result;
}

std::vector<glm::vec3> Scene::renderTest() const {
    std::vector<glm::vec3> result;
    int width = camera->get_image_width();
    int height = camera->get_image_height();
    for (int y = 0; y < height; y++) {
        std::cout << "Scanline: " << y << std::endl;
        for (int x = 0; x < width; x++) {
            glm::vec3 avgColor(0, 0, 0);
            for (int rpp = 0; rpp < ray_per_pixel; rpp++) {
                // Get ray orig and dir
                // Use a random offset for antialiasing
                glm::vec2 offset = glm::vec2(generateRandomOffset().x, generateRandomOffset().y);

                // 1. Get UV coordinates (-0.5 to 0.5)
                glm::vec2 uv = glm::vec2(
                    (static_cast<float>(x) + offset.x) / width - 0.5f,
                    (static_cast<float>(y) + offset.y) / height - 0.5f
                );

                // 2. Scale by viewport dimensions
                // viewportWidth = viewParams.x, viewportHeight = viewParams.y, focalLength = viewParams.z
                glm::vec3 viewPointLocal = glm::vec3(uv.x, -uv.y, -1) * camera->viewParams;

                // 3. Transform local direction to world space
                glm::vec3 rayDir = glm::vec3(camera->view * glm::vec4(viewPointLocal, 0.0f));

                // The ray's origin is the camera's world-space position
                glm::vec3 rayOrig = camera->get_center();

                rayDir = glm::normalize(rayDir);
                Ray ray(rayOrig, rayDir);

                avgColor += trace(ray);
            }
            avgColor /= ray_per_pixel;
            avgColor = glm::clamp(avgColor, 0.0f, 1.0f);

            result.push_back(avgColor);
        }
    }
    return result;
}

glm::vec3 Scene::trace(Ray& ray) const {
    glm::vec3 finalColor = glm::vec3(0, 0, 0);
    glm::vec3 rayColor = glm::vec3(1.0f, 1.0f, 1.0f);
    for (int mrb = 0 ; mrb < max_ray_bounce; mrb++) {
        HitInfo hit = intersect(ray);
        if (!hit.hit) {
            //finalColor += colorPixel(ray) * rayColor;
            break;
        }
        glm::vec3 newPos = ray.at(hit.hitDist);
        glm::vec3 newDir = glm::normalize(hit.normal + random_unit_vector());

        glm::vec3 emittedLight = hit.material.getEmissionColor() * hit.material.getEmissionStrength();
        //float lightStrength = glm::dot(hit.normal, ray.direction());
        finalColor += emittedLight * rayColor;
        rayColor *= hit.material.getColor();
        ray.setDirection(newDir);
        ray.setOrigin(newPos);
    }
    return glm::clamp(finalColor, 0.0f, 1.0f);
}

HitInfo Scene::intersect(Ray& ray) const {
    HitInfo bestHit;
    for (const auto& object : objects) {
        object->intersect(ray, bestHit);
    }
    return bestHit;
}

glm::vec3 Scene::lerp(glm::vec3 a, glm::vec3 b, float t) const {
    glm::vec3 u = a * (1.0f - t);
    glm::vec3 v = b * t;
    return u + v;
}



