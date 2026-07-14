#pragma once

#include <filesystem>
#include <yaml-cpp/yaml.h>

#include "core/project.hpp"

namespace YAML {
template <> struct convert<k2::Project> {
    static Node encode(const k2::Project& project) {
        Node node;
        node["version"] = "0.0.1";
        node["name"] = project.name;
        node["main_scene"] = std::filesystem::relative(project.main_scene, project.root).generic_string();
        node["assets"] = project.assets_node.IsDefined() && !project.assets_node.IsNull()
            ? project.assets_node
            : Node { NodeType::Map };
        return node;
    }

    // Decoding lives in k2::Project::load: the asset registry must be loaded
    // and paths anchored to the project file, which convert<> cannot do.
};
}
