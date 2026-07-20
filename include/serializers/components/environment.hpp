#pragma once
#include "components/environment.hpp"
#include "serializers/utils.hpp" // IWYU pragma: keep
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::Environment> {
    static Node encode(const k2::Environment& environment) {
        YAML::Node node;
        node["AmbientColor"] = environment.ambient_color;
        node["AmbientIntensity"] = environment.ambient_intensity;
        node["ClearColor"] = environment.clear_color;
        node["Bloom"] = environment.bloom;
        node["BloomIntensity"] = environment.bloom_intensity;
        node["BloomThreshold"] = environment.bloom_threshold;
        return node;
    }

    static bool decode(const Node& node, k2::Environment& environment) {
        if (!node.IsMap()) {
            return false;
        }
        environment.ambient_color = node["AmbientColor"].as<glm::vec3>(glm::vec3 { 1.0f });
        environment.ambient_intensity = node["AmbientIntensity"].as<float>(1.0f);
        environment.clear_color = node["ClearColor"].as<glm::vec4>(glm::vec4 { 0.0f, 0.0f, 0.0f, 1.0f });
        environment.bloom = node["Bloom"].as<bool>(true);
        environment.bloom_intensity = node["BloomIntensity"].as<float>(1.0f);
        environment.bloom_threshold = node["BloomThreshold"].as<float>(1.0f);
        return true;
    }
};
}
