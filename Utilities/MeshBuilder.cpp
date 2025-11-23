//
// Created by Samuel on 11/13/2025.
//

#include "MeshBuilder.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/gtx/hash.hpp>

std::map<std::string, std::shared_ptr<Mesh>> MeshBuilder::meshCache;

std::shared_ptr<Mesh> MeshBuilder::buildMesh(const std::string &meshFile) {
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;

    std::vector<glm::vec3> tempVertices;
    std::vector<glm::vec2> tempUvs;
    std::vector<glm::vec3> tempNormals;

    FILE* file = fopen(meshFile.c_str(), "r");
    if (file == NULL) {
        throw std::runtime_error("Failed to open file " + meshFile);
    }

    while (1) {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF) {
            break;
        }
        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f", &vertex.x, &vertex.y, &vertex.z);
            tempVertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f", &uv.x, &uv.y);
            tempUvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f", &normal.x, &normal.y, &normal.z);
            tempNormals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            std::string vertex1, vertex2, vertex3;
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d",
                &vertexIndex[0],
                &uvIndex[0],
                &normalIndex[0],
                &vertexIndex[1],
                &uvIndex[1],
                &normalIndex[1],
                &vertexIndex[2],
                &uvIndex[2],
                &normalIndex[2]);
            if (matches != 9) {
                throw std::runtime_error("Failed to parse mesh file: <Incorrect face format> " + meshFile);
            }
            vertexIndices.emplace_back(vertexIndex[0] - 1);
            vertexIndices.emplace_back(vertexIndex[1] - 1);
            vertexIndices.emplace_back(vertexIndex[2] - 1);
            uvIndices.emplace_back(uvIndex[0] - 1);
            uvIndices.emplace_back(uvIndex[1] - 1);
            uvIndices.emplace_back(uvIndex[2] - 1);
            normalIndices.emplace_back(normalIndex[0] - 1);
            normalIndices.emplace_back(normalIndex[1] - 1);
            normalIndices.emplace_back(normalIndex[2] - 1);
        }
    }

    return std::make_shared<Mesh>(std::move(tempVertices), std::move(tempNormals), std::move(tempUvs),
        std::move(vertexIndices), std::move(normalIndices), std::move(uvIndices));

}