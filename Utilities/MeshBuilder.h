//
// Created by Samuel on 11/13/2025.
//

#ifndef MESHBUILDER_H
#define MESHBUILDER_H

#include <map>

#include "../ObjectClasses/Mesh.h"

#include <memory>
#include <assimp/mesh.h>
#include <assimp/scene.h>


class MeshBuilder {
public:
    static std::shared_ptr<Mesh> getMesh(const std::string& filename) {
        if (meshCache.count(filename)) {
            return meshCache.at(filename);
        }

        std::shared_ptr<Mesh> newMesh = buildMesh(filename);
        meshCache[filename] = newMesh;

        return newMesh;
    }

private:
    static std::map<std::string, std::shared_ptr<Mesh>> meshCache;
    static std::shared_ptr<Mesh> buildMesh(const std::string& meshFile);
};

#endif //MESHBUILDER_H
