#pragma once
#include "components/tag.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::TagComponent> {
    static Node encode(const k2::TagComponent& tag_component) { return YAML::Node(std::string(tag_component.str())); }

    static bool decode(const Node& node, k2::TagComponent& tag_component) {
        auto tag = node.as<std::string>();
        auto length = std::min(tag.size(), tag_component.tag.size() - 1);
        std::memcpy(tag_component.tag.data(), tag.c_str(), length);
        tag_component.tag[length] = 0;
        return true;
    }
};
}
