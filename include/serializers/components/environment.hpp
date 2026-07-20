#pragma once
#include "components/environment.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::Environment> {
    static Node encode(const k2::Environment& environment) {
        YAML::Node node;
        node["Bloom"] = environment.bloom;
        node["BloomIntensity"] = environment.bloom_intensity;
        node["BloomThreshold"] = environment.bloom_threshold;
        return node;
    }

    static bool decode(const Node& node, k2::Environment& environment) {
        if (!node.IsMap()) {
            return false;
        }
        environment.bloom = node["Bloom"].as<bool>(true);
        environment.bloom_intensity = node["BloomIntensity"].as<float>(1.0f);
        environment.bloom_threshold = node["BloomThreshold"].as<float>(1.0f);
        return true;
    }
};
}
