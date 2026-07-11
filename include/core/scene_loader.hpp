#pragma once

#include <filesystem>

#include <yaml-cpp/yaml.h>

#include "asset/asset_registry.hpp"
#include "core/resources.hpp"
#include "core/scene.hpp"

namespace k2 {
struct SceneLoader {
    static Scene load(const std::filesystem::path& path, ResourceManager& resources, const AssetRegistry& assets);

    static Scene load(const YAML::Node& node, ResourceManager& resources, const AssetRegistry& assets);
};
}
