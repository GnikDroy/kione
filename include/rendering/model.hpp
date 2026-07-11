#pragma once

#include <filesystem>
#include <vector>

#include "core/resources.hpp"
#include "mesh.hpp"

namespace k2 {
class Model {
    std::vector<Mesh> meshes;
    std::filesystem::path path;

public:
    Model() = default;

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    Model(const std::filesystem::path& path, ResourceManager& resources);

    void draw(const Program& program, ResourceManager& resources);
};
}
