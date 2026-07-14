#pragma once

#include <expected>
#include <filesystem>
#include <string>

#include <yaml-cpp/yaml.h>

#include "asset/asset_registry.hpp"

namespace k2 {
struct Project {
    std::string name;
    std::filesystem::path file;
    std::filesystem::path root;
    std::filesystem::path main_scene;
    YAML::Node assets_node;
    AssetRegistry assets;

    static Project load(const std::filesystem::path& project_file);

    void reload_assets();

    [[nodiscard]] std::expected<void, std::string> save() const;
};
}
