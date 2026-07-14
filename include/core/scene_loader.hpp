#pragma once

#include <expected>
#include <filesystem>
#include <string>

#include <yaml-cpp/yaml.h>

#include "asset/asset_registry.hpp"
#include "core/resources.hpp"
#include "core/scene.hpp"

namespace k2 {
struct SceneLoader {
    [[nodiscard]] static std::expected<Scene, std::string> load(
        const std::filesystem::path& path, ResourceManager& resources, const AssetRegistry& assets) noexcept;

    [[nodiscard]] static std::expected<Scene, std::string> load(
        const YAML::Node& node, ResourceManager& resources, const AssetRegistry& assets) noexcept;
};
}
