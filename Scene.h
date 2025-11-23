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
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(int ray_per_pixel, int max_ray_bounce, Camera* camera);
    ~Scene() {}
    std::vector<glm::vec3> render() const;
    std::vector<glm::vec3> renderTest() const;
    glm::vec3 trace(Ray& ray) const;
    HitInfo intersectScene(Ray& ray) const;

    [[nodiscard]] std::vector<SphereObject> getSpheres() const {
        return spheres;
    }
    [[nodiscard]] std::vector<MeshObject> getMeshes() const {
        return meshes;
    }
private:
    // std::vector<std::unique_ptr<Light>> lights;
    int ray_per_pixel;
    int max_ray_bounce;
    Camera* camera;
    glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t) const;

    std::vector<SphereObject> spheres;
    std::vector<MeshObject> meshes;

    void buildDefaultScene();
};



#endif //SCENE_H
