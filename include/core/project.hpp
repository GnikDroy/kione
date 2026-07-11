#pragma once

#include <filesystem>
#include <format>
#include <stdexcept>
#include <string>
#include <yaml-cpp/yaml.h>

#include "asset/asset_registry.hpp"

namespace k2 {
struct Project {
    std::string name;
    std::filesystem::path root;
    std::filesystem::path main_scene;
    AssetRegistry assets;

    static Project load(const std::filesystem::path& project_file) {
        return load(YAML::LoadFile(project_file.string()), std::filesystem::absolute(project_file).parent_path());
    }

    static Project load(const YAML::Node& node, const std::filesystem::path& root) {
        if (!node.IsMap()) {
            throw std::runtime_error("A project must be a map.");
        }

        Project project;
        project.name = node["name"].as<std::string>("");
        project.root = root;
        project.main_scene = root / node["main_scene"].as<std::string>();

        // Asset urls resolve relative to the working directory, so the bundle path
        // is rebased onto it
        auto bundle_path = std::filesystem::relative(root / node["assets"].as<std::string>());
        project.assets = AssetRegistryLoader::load({
            .url = std::format("file:///{}", bundle_path.generic_string()),
            .type = Asset::Type::AssetBundle,
        });
        return project;
    }
};
}
