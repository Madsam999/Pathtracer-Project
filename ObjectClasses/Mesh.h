//
// Created by Samuel on 11/12/2025.
//

#ifndef MESH_H
#define MESH_H
#include <iostream>
#include <ostream>
#include <vector>

#include <glm/glm.hpp>

struct alignas(16) Triangle {
    glm::vec4 positionA, positionB, positionC, normalA, normalB, normalC;
    // TODO: ADD TEX COORDS
};

class Mesh {
public:

    [[nodiscard]] const std::vector<glm::vec3>& getVertices() const {
        return vertices;
    }
    [[nodiscard]] const std::vector<glm::vec3>& getNormals() const {
        return normals;
    }
    [[nodiscard]] const std::vector<glm::vec2>& getUvs() const {
        return texCoords;
    }
    [[nodiscard]] const std::vector<unsigned int>& getVertIndices() const {
        return vertIndices;
    }
    [[nodiscard]] const std::vector<unsigned int>& getNormalIndices() const {
        return normalIndices;
    }
    [[nodiscard]] const std::vector<unsigned int>& getTexIndices() const {
        return texIndices;
    }
    [[nodiscard]] const std::vector<Triangle>& getTriangles() const {
        return triangles;
    }
    [[nodiscard]] const glm::vec3& getMin() const {
        return minBound;
    }
    [[nodiscard]] const glm::vec3& getMax() const {
        return maxBound;
    }

    Mesh(std::vector<glm::vec3> vertices,
        std::vector<glm::vec3> normals,
        std::vector<glm::vec2> texCoords,
        std::vector<unsigned int> indices,
        std::vector<unsigned int> normalIndices,
        std::vector<unsigned int> texIndices) :
    vertices(std::move(vertices)),
    normals(std::move(normals)),
    texCoords(std::move(texCoords)),
    vertIndices(std::move(indices)),
    normalIndices(std::move(normalIndices)),
    texIndices(std::move(texIndices)) {
        setupForGPUTransfer();
    }




private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<unsigned int> vertIndices;
    std::vector<unsigned int> normalIndices;
    std::vector<unsigned int> texIndices;

    std::vector<Triangle> triangles;

    glm::vec3 minBound, maxBound;

    void setupForGPUTransfer();



    friend class MeshBuilder;
};



#endif //MESH_H
