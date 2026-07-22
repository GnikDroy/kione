#pragma once

#include <expected>
#include <string>
#include <string_view>

#include <yaml-cpp/yaml.h>

#include "asset/asset_registry.hpp"
#include "core/resources.hpp"
#include "core/scene.hpp"

namespace k2 {
struct SceneLoader {
    [[nodiscard]] static std::expected<Scene, std::string> load(
        std::string_view name, ResourceManager& resources, const AssetRegistry& assets) noexcept;

    [[nodiscard]] static std::expected<Scene, std::string> load(
        const YAML::Node& node, ResourceManager& resources, const AssetRegistry& assets) noexcept;

    static void load_resources(entt::registry& registry, ResourceManager& resources, const AssetRegistry& assets);
};
}
