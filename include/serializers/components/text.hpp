#pragma once
#include <string>

#include "components/text.hpp"
#include "serializers/asset/asset_handle.hpp"
#include "serializers/utils.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::TextComponent> {
    static Node encode(const k2::TextComponent& text) {
        YAML::Node node;
        node["Font"] = text.font;
        node["Text"] = text.text;
        node["Size"] = text.size;
        node["Color"] = text.color;
        return node;
    }

    static bool decode(const Node& node, k2::TextComponent& text) {
        if (!node.IsMap()) {
            return false;
        }
        text.font = node["Font"].as<k2::AssetHandle>();
        text.text = node["Text"].as<std::string>();
        text.size = node["Size"].as<float>();
        text.color = node["Color"].as<glm::vec4>();
        return true;
    }
};
}
