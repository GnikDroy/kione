#pragma once
#include "components/audio.hpp"
#include "serializers/asset/asset_handle.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::AudioSourceComponent> {
    static Node encode(const k2::AudioSourceComponent& source) {
        YAML::Node node;
        node["Clip"] = source.clip;
        node["Volume"] = source.volume;
        node["Pitch"] = source.pitch;
        node["Looping"] = source.looping;
        node["PlayOnCreate"] = source.play_on_create;
        return node;
    }

    static bool decode(const Node& node, k2::AudioSourceComponent& source) {
        if (!node.IsMap()) {
            return false;
        }
        source.clip = node["Clip"].as<k2::AssetHandle>();
        source.volume = node["Volume"].as<float>();
        source.pitch = node["Pitch"].as<float>();
        source.looping = node["Looping"].as<bool>();
        source.play_on_create = node["PlayOnCreate"].as<bool>();
        return true;
    }
};
}
