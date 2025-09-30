//
// Created by Samuel on 2025-07-30.
//

#ifndef SCENE_H
#define SCENE_H
#include <vector>
#include <memory>

#include "Camera.h"
#include "Object.h"
#include "Sphere.h"
#include "Utilities/RandomUtilities.cpp"
// #include "Light.h"


class Scene {
public:
    Scene() {}
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(int ray_per_pixel, int max_ray_bounce, Camera* camera) : ray_per_pixel(ray_per_pixel), max_ray_bounce(max_ray_bounce), camera(camera) {
        Material red(glm::vec3(1.0f, 0.7f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f);
        Material green(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f);
        Material blue(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 1.0f);
        Material light(glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.0f);
        objects.push_back(std::make_unique<Sphere>(red, glm::vec3(0.0f, -0.25f, -1.5f), 0.5f));
        objects.push_back(std::make_unique<Sphere>(green, glm::vec3(1.5f, 0.0f, -1.0f), 1.0f));
        objects.push_back(std::make_unique<Sphere>(light, glm::vec3(0.0f, 6.0f, 0.0f), 5.0f));
        objects.push_back(std::make_unique<Sphere>(blue, glm::vec3(0.0f, -20.5f, 1.0f), 20.0f));

        Sphere s1(red, glm::vec3(0.0f, -0.25f, -1.5f), 0.5f);
        Sphere s2(green, glm::vec3(1.5f, 0.0f, -1.0f), 1.0f);
        Sphere s3(light, glm::vec3(0.0f, 6.0f, 0.0f), 5.0f);
        Sphere s4(blue, glm::vec3(0.0f, -20.5f, 1.0f), 20.0f);

        sceneSpheres.push_back(s1);
        sceneSpheres.push_back(s2);
        sceneSpheres.push_back(s3);
        sceneSpheres.push_back(s4);
    }
    ~Scene() {}
    std::vector<glm::vec3> render() const;
    std::vector<glm::vec3> renderTest() const;
    glm::vec3 trace(Ray& ray) const;
    HitInfo intersect(Ray& ray) const;
    std::vector<Sphere> getSpheres() { return sceneSpheres; }
private:
    std::vector<std::unique_ptr<Object>> objects;
    // std::vector<std::unique_ptr<Light>> lights;
    int ray_per_pixel;
    int max_ray_bounce;
    Camera* camera;
    glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t) const;
    std::vector<Sphere> sceneSpheres;
};



#endif //SCENE_H
