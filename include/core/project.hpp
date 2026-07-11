#pragma once

#include <filesystem>
#include <string>

#include <yaml-cpp/yaml.h>

#include "asset/asset_registry.hpp"

namespace k2 {
struct Project {
    std::string name;
    std::filesystem::path root;
    std::filesystem::path main_scene;
    AssetRegistry assets;

    static Project load(const std::filesystem::path& project_file);

    static Project load(const YAML::Node& node, const std::filesystem::path& root);
};
}
