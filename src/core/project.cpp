#include "core/project.hpp"

#include "serializers/core/project.hpp" // IWYU pragma: keep

#include <format>
#include <fstream>
#include <stdexcept>

namespace k2 {

static AssetRegistry load_registry(const std::filesystem::path& project_file) {
    auto bundle_path = std::filesystem::relative(project_file);
    return AssetRegistryLoader::load({
        .url = std::format("file:///{}", bundle_path.generic_string()),
        .type = Asset::Type::AssetBundle,
    });
}

Project Project::load(const std::filesystem::path& project_file) {
    auto node = YAML::LoadFile(project_file.string());
    if (!node.IsMap()) {
        throw std::runtime_error("A project must be a map.");
    }

    Project project;
    project.file = std::filesystem::absolute(project_file);
    project.root = project.file.parent_path();
    project.name = node["name"].as<std::string>("");
    project.main_scene = project.root / node["main_scene"].as<std::string>();
    project.assets_node = node["assets"];
    project.assets = load_registry(project.file);
    return project;
}

void Project::reload_assets() {
    auto node = YAML::LoadFile(file.string());
    assets_node = node["assets"];
    assets = load_registry(file);
}

void Project::save() const {
    if (file.empty()) {
        throw std::runtime_error("Project has no backing file to save to.");
    }

    std::ofstream out { file };
    out << YAML::Node { *this } << "\n";
    if (!out) {
        throw std::runtime_error(std::format("Failed to write project file: {}", file.string()));
    }
}

}
