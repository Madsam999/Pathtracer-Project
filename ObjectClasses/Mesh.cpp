//
// Created by Samuel on 11/12/2025.
//

#include "Mesh.h"


void Mesh::setupForGPUTransfer() {
    auto vertices = getVertices();
    auto normals = getNormals();
    auto uvs = getUvs();

    auto vertexIndices = getVertIndices();
    auto normalIndices = getNormalIndices();
    auto uvIndices = getTexIndices();

    for (int i = 0; i < vertexIndices.size(); i += 3) {
        int v_idx_A = vertexIndices[i + 0];
        int v_idx_B = vertexIndices[i + 1];
        int v_idx_C = vertexIndices[i + 2];

        int n_idx_A = normalIndices[i + 0];
        int n_idx_B = normalIndices[i + 1];
        int n_idx_C = normalIndices[i + 2];

        int uv_idx_A = uvIndices[i + 0];
        int uv_idx_B = uvIndices[i + 1];
        int uv_idx_C = uvIndices[i + 2];

        glm::vec3 v1 = vertices[v_idx_A];
        glm::vec3 v2 = vertices[v_idx_B];
        glm::vec3 v3 = vertices[v_idx_C];

        glm::vec3 n1 = normals[n_idx_A];
        glm::vec3 n2 = normals[n_idx_B];
        glm::vec3 n3 = normals[n_idx_C];
        Triangle triangle{};
        triangle.positionA = glm::vec4(v1, 1);
        triangle.positionB = glm::vec4(v2, 1);
        triangle.positionC = glm::vec4(v3, 1);
        triangle.normalA = glm::vec4(n1, 0);
        triangle.normalB = glm::vec4(n2, 0);
        triangle.normalC = glm::vec4(n3, 0);
        triangles.push_back(triangle);
    }

    glm::vec3 min = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    glm::vec3 max = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    for (glm::vec3 v : vertices) {
        min.x = std::min(min.x, v.x);
        min.y = std::min(min.y, v.y);
        min.z = std::min(min.z, v.z);
        max.x = std::max(max.x, v.x);
        max.y = std::max(max.y, v.y);
        max.z = std::max(max.z, v.z);
    }

    minBound = min;
    maxBound = max;
}
