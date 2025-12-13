//
// Created by Samuel on 2025-07-30.
//

#ifndef SCENE_H
#define SCENE_H
#include <vector>
#include <memory>

#include "Camera.h"
#include "ObjectClasses/MeshObject.h"
#include "ObjectClasses/SphereObject.h"
#include "ObjectClasses/SceneObjects.h"
#include "Utilities/RandomUtilities.cpp"
// #include "Light.h"


class Scene {
public:
    Scene() {}
    Scene(int ray_per_pixel, int max_ray_bounce, Camera* camera);
    ~Scene() {}
    std::vector<glm::vec3> render();
    std::vector<glm::vec3> renderTest();
    glm::vec3 trace(Ray& ray);
    HitInfo intersectScene(Ray& ray);

    [[nodiscard]] std::vector<SphereObject>& getSpheres() {
        return spheres;
    }
    [[nodiscard]] std::vector<MeshObject>& getMeshes() {
        return meshes;
    }

    [[nodiscard]] int ray_per_pixel1() const {
        return ray_per_pixel;
    }

    [[nodiscard]] int max_ray_bounce1() const {
        return max_ray_bounce;
    }

    void buildDefaultScene();
private:
    // std::vector<std::unique_ptr<Light>> lights;
    int ray_per_pixel;
    int max_ray_bounce;
    Camera* camera;
    glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t) const;

    std::vector<SphereObject> spheres;
    std::vector<MeshObject> meshes;
};



#endif //SCENE_H
