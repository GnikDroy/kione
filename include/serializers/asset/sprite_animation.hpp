#pragma once
#include <yaml-cpp/yaml.h>

#include "rendering/sprite_animation.hpp"
#include "serializers/asset/asset_handle.hpp"
#include "serializers/utils.hpp" // IWYU pragma: keep

namespace YAML {
template <> struct convert<k2::SpriteAnimation::Frame> {
    static Node encode(const k2::SpriteAnimation::Frame& frame) {
        Node node;
        node["uv"] = frame.uv;
        node["duration"] = frame.duration;
        node["color"] = frame.color;
        return node;
    }

    static bool decode(const Node& node, k2::SpriteAnimation::Frame& frame) {
        if (!node.IsMap()) {
            return false;
        }
        frame.uv = node["uv"].as<k2::Rectf>();
        frame.duration = node["duration"].as<float>(0.1f);
        if (node["color"].IsDefined()) {
            frame.color = node["color"].as<glm::vec4>();
        }
        return true;
    }
};

template <> struct convert<k2::SpriteAnimation> {
    static Node encode(const k2::SpriteAnimation& animation) {
        Node node;
        node["texture"] = animation.texture;
        node["loop"] = animation.loop;
        node["frames"] = animation.frames;
        return node;
    }

    static bool decode(const Node& node, k2::SpriteAnimation& animation) {
        if (!node.IsMap()) {
            return false;
        }
        animation.texture = node["texture"].as<k2::AssetHandle>();
        animation.loop = node["loop"].as<bool>(true);
        animation.frames = node["frames"].as<std::vector<k2::SpriteAnimation::Frame>>();
        return true;
    }
};
}
