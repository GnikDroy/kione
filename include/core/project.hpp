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
    std::string main_scene;
    YAML::Node assets_node;
    AssetRegistry assets;

    [[nodiscard]] static std::expected<Project, std::string> load(const std::filesystem::path& project_file) noexcept;

    [[nodiscard]] std::expected<void, std::string> reload_assets() noexcept;

    [[nodiscard]] std::expected<void, std::string> save() const;
};
}
