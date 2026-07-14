#pragma once
#include "components/animation.hpp"
#include "serializers/asset/asset_handle.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::AnimationComponent> {
    static Node encode(const k2::AnimationComponent& animation) {
        YAML::Node node;
        node["Clip"] = animation.clip;
        node["Speed"] = animation.speed;
        node["Playing"] = animation.playing;
        return node;
    }

    static bool decode(const Node& node, k2::AnimationComponent& animation) {
        if (!node.IsMap()) {
            return false;
        }
        animation.clip = node["Clip"].as<k2::AssetHandle>();
        animation.speed = node["Speed"].as<float>();
        animation.playing = node["Playing"].as<bool>();
        return true;
    }
};
}
