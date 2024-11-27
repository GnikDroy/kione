#pragma once
#include "components/transform.hpp"
#include "serializers/utils.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::TransformComponent> {
    static Node encode(const k2::TransformComponent& transform) {
        YAML::Node node;
        node["Translation"] = transform.translation;
        node["Orientation"] = transform.orientation;
        node["Scale"] = transform.scale;
        return node;
    }

    static bool decode(const Node& node, k2::TransformComponent& transform) {
        transform.translation = node["Translation"].as<glm::vec3>();
        transform.orientation = node["Orientation"].as<glm::quat>();
        transform.scale = node["Scale"].as<glm::vec3>();
        return true;
    }
};
}