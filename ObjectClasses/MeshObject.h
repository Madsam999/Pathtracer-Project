//
// Created by Samuel on 11/12/2025.
//

#ifndef MESHOBJECT_H
#define MESHOBJECT_H
#include "SceneObjects.h"
#include "Mesh.h"


class MeshObject : public SceneObject{
public:
    MeshObject(
        glm::vec3 position,
        glm::vec3 rotation,
        glm::vec3 scale,
        std::shared_ptr<Material> material
    ) :
    SceneObject(
        position,
        rotation,
        scale,
        std::move(material)
    ) {
        printf("Mesh Object with ID %llu created!", this->getObjectID());
    }

    void localIntersect(Ray& ray, HitInfo& hit_info) const override;

    void setMesh(std::shared_ptr<Mesh> meshData) {
        mesh = std::move(meshData);
    }

    [[nodiscard]] std::shared_ptr<Mesh> getMesh() const {
        return mesh;
    }
private:
    std::shared_ptr<Mesh> mesh;

    void triangleIntersect(Ray& ray, HitInfo& hit_info, std::vector<glm::vec3> triangleVerts, std::vector<glm::vec3> triangleNorms, std::vector<glm::vec2> triangleUVs) const;
};



#endif //MESHOBJECT_H
