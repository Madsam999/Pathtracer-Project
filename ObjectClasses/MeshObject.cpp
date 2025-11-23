//
// Created by Samuel on 11/12/2025.
//

#include "MeshObject.h"

void MeshObject::localIntersect(Ray &ray, HitInfo &hit_info) const {
    if (!mesh) {
        printf("<Mesh Object has no mesh data associated>");
        return;
    }

    const auto& verts = mesh->getVertices();
    const auto& norms = mesh->getNormals();
    const auto& uvs = mesh->getUvs();
    const auto& indices = mesh->getVertIndices();

    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i0 = indices[i + 0];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        std::vector<glm::vec3> verts_tmp = {verts[i0], verts[i1], verts[i2]};
        std::vector<glm::vec2> uvs_tmp = {uvs[i0], uvs[i1], uvs[i2]};
        std::vector<glm::vec3> normals_tmp = {norms[i0], norms[i1], norms[i2]};


        triangleIntersect(ray, hit_info, verts_tmp, normals_tmp, uvs_tmp);
    }
}

void MeshObject::triangleIntersect(Ray &ray, HitInfo &hit_info, std::vector<glm::vec3> triangleVerts, std::vector<glm::vec3> triangleNorms, std::vector<glm::vec2> triangleUVs) const {
    glm::vec3 edgeAB = triangleVerts[1] - triangleVerts[0];
    glm::vec3 edgeAC = triangleVerts[2] - triangleVerts[0];
    glm::vec3 normal = glm::cross(edgeAB, edgeAC);
    glm::vec3 ao = ray.origin() - triangleVerts[0];
    glm::vec3 dao = glm::cross(ao, ray.direction());

    float determinant = -glm::dot(ray.direction(), normal);
    float invDeterminant = 1.0f / determinant;

    float dst = glm::dot(ao, normal) * invDeterminant;
    float u = glm::dot(edgeAC, dao) * invDeterminant;
    float v = glm::dot(edgeAB, dao) * invDeterminant;
    float w = 1 - u - v;

    glm::vec3 norm1 = triangleNorms[0];
    glm::vec3 norm2 = triangleNorms[1];
    glm::vec3 norm3 = triangleNorms[2];

    hit_info.hit = determinant >= 1e-6 && dst >= 0 && u >= 0 && v >= 0 && w >= 0;
    hit_info.normal = glm::normalize(norm1 * u + norm2 * v + norm3 * w);
    hit_info.hitDist = dst;
    hit_info.material = getMaterial();
}


