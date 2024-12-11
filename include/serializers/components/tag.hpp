#pragma once
#include "components/tag.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::TagComponent> {
    static Node encode(const k2::TagComponent& tag_component) { return YAML::Node(std::string(tag_component.str())); }

    static bool decode(const Node& node, k2::TagComponent& tag_component) {
        auto tag = node.as<std::string>();
        std::memcpy(tag_component.tag.data(), tag.c_str(), std::min(tag.size(), tag_component.tag.size()));
        tag_component.tag[std::min(tag_component.tag.size(), tag.size())] = 0;
        return true;
    }
};
}
