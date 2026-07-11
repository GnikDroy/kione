#pragma once
#include "components/tag.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::TagComponent> {
    static Node encode(const k2::TagComponent& tag_component) { return Node(tag_component.tag); }

    static bool decode(const Node& node, k2::TagComponent& tag_component) {
        if (!node.IsScalar()) {
            return false;
        }
        tag_component.tag = node.Scalar();
        return true;
    }
};
}
