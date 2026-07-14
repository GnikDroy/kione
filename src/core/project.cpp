#include "core/project.hpp"

#include <format>
#include <fstream>
#include <stdexcept>

namespace k2 {

Project Project::load(const std::filesystem::path& project_file) {
    auto project = load(YAML::LoadFile(project_file.string()), std::filesystem::absolute(project_file).parent_path());
    project.file = std::filesystem::absolute(project_file);
    return project;
}

Project Project::load(const YAML::Node& node, const std::filesystem::path& root) {
    if (!node.IsMap()) {
        throw std::runtime_error("A project must be a map.");
    }

    Project project;
    project.name = node["name"].as<std::string>("");
    project.root = root;
    project.main_scene = root / node["main_scene"].as<std::string>();
    project.assets_file = root / node["assets"].as<std::string>();

    // Asset urls resolve relative to the working directory, so the bundle path
    // is rebased onto it
    auto bundle_path = std::filesystem::relative(project.assets_file);
    project.assets = AssetRegistryLoader::load({
        .url = std::format("file:///{}", bundle_path.generic_string()),
        .type = Asset::Type::AssetBundle,
    });
    return project;
}

void Project::save() const {
    if (file.empty()) {
        throw std::runtime_error("Project has no backing file to save to.");
    }
    std::ofstream out { file };
    out << std::format("version: 0.0.1\nname: {}\nassets: {}\nmain_scene: {}\n", name,
        std::filesystem::relative(assets_file, root).generic_string(),
        std::filesystem::relative(main_scene, root).generic_string());
    if (!out) {
        throw std::runtime_error(std::format("Failed to write project file: {}", file.string()));
    }
}

}
