#include "core/project.hpp"

#include <format>
#include <stdexcept>

namespace k2 {

Project Project::load(const std::filesystem::path& project_file) {
    return load(YAML::LoadFile(project_file.string()), std::filesystem::absolute(project_file).parent_path());
}

Project Project::load(const YAML::Node& node, const std::filesystem::path& root) {
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

}
