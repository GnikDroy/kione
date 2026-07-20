#pragma once
#include "components/light.hpp"
#include "serializers/asset/asset_handle.hpp"
#include "serializers/utils.hpp" // IWYU pragma: keep
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::PointLight> {
    static Node encode(const k2::PointLight& light) {
        YAML::Node node;
        node["Color"] = light.color;
        node["Intensity"] = light.intensity;
        node["Radius"] = light.radius;
        return node;
    }

    static bool decode(const Node& node, k2::PointLight& light) {
        if (!node.IsMap()) {
            return false;
        }
        light.color = node["Color"].as<glm::vec3>();
        light.intensity = node["Intensity"].as<float>();
        light.radius = node["Radius"].as<float>();
        return true;
    }
};

template <> struct convert<k2::SpotLight> {
    static Node encode(const k2::SpotLight& light) {
        YAML::Node node;
        node["Color"] = light.color;
        node["Intensity"] = light.intensity;
        node["Radius"] = light.radius;
        node["InnerAngle"] = light.inner_angle;
        node["OuterAngle"] = light.outer_angle;
        return node;
    }

    static bool decode(const Node& node, k2::SpotLight& light) {
        if (!node.IsMap()) {
            return false;
        }
        light.color = node["Color"].as<glm::vec3>();
        light.intensity = node["Intensity"].as<float>();
        light.radius = node["Radius"].as<float>();
        light.inner_angle = node["InnerAngle"].as<float>();
        light.outer_angle = node["OuterAngle"].as<float>();
        return true;
    }
};

template <> struct convert<k2::SpriteLight> {
    static Node encode(const k2::SpriteLight& light) {
        YAML::Node node;
        node["Texture"] = light.texture;
        node["Color"] = light.color;
        node["Intensity"] = light.intensity;
        return node;
    }

    static bool decode(const Node& node, k2::SpriteLight& light) {
        if (!node.IsMap()) {
            return false;
        }
        light.texture = node["Texture"].as<k2::AssetHandle>();
        light.color = node["Color"].as<glm::vec3>();
        light.intensity = node["Intensity"].as<float>();
        return true;
    }
};
}
