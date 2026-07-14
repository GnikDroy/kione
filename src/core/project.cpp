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

std::expected<Project, std::string> Project::load(const std::filesystem::path& project_file) noexcept {
    try {
        auto node = YAML::LoadFile(project_file.string());
        if (!node.IsMap()) {
            return std::unexpected("A project must be a map.");
        }

        Project project;
        project.file = std::filesystem::absolute(project_file);
        project.root = project.file.parent_path();
        project.name = node["name"].as<std::string>("");
        project.main_scene = project.root / node["main_scene"].as<std::string>();
        project.assets_node = node["assets"];
        project.assets = load_registry(project.file);
        return project;
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

std::expected<void, std::string> Project::reload_assets() noexcept {
    try {
        auto node = YAML::LoadFile(file.string());
        assets_node = node["assets"];
        assets = load_registry(file);
        return {};
    } catch (const std::exception& e) {
        return std::unexpected(e.what());
    }
}

std::expected<void, std::string> Project::save() const {
    if (file.empty()) {
        return std::unexpected("Project has no backing file to save to.");
    }

    std::ofstream out { file };
    out << YAML::Node { *this } << "\n";
    if (!out) {
        return std::unexpected(std::format("Failed to write project file: {}", file.string()));
    }
    return {};
}

}
