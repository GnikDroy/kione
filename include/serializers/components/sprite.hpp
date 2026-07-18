#pragma once
#include "components/sprite.hpp"
#include "serializers/asset/asset_handle.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::SpriteComponent> {
    static Node encode(const k2::SpriteComponent& sprite) {
        YAML::Node node;
        node["Texture"] = sprite.texture;
        node["Color"] = sprite.color;
        node["UvRect"] = sprite.uv_rect;
        node["Unlit"] = sprite.unlit;
        return node;
    }

    static bool decode(const Node& node, k2::SpriteComponent& sprite) {
        if (!node.IsMap()) {
            return false;
        }
        sprite.texture = node["Texture"].as<k2::AssetHandle>();
        sprite.color = node["Color"].as<glm::vec4>();
        sprite.uv_rect = node["UvRect"].as<k2::Rect<float>>();
        sprite.unlit = node["Unlit"].as<bool>(false);
        return true;
    }
};
}
