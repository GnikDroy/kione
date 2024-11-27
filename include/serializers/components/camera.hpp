#pragma once
#include "components/camera.hpp"
#include "serializers/utils.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::Camera::OrthographicTraits> {
    static Node encode(const k2::Camera::OrthographicTraits& orthographic_traits) {
        YAML::Node node;
        node["Left"] = orthographic_traits.left;
        node["Right"] = orthographic_traits.right;
        node["Top"] = orthographic_traits.top;
        node["Bottom"] = orthographic_traits.bottom;
        node["FarClip"] = orthographic_traits.far_clip;
        node["NearClip"] = orthographic_traits.near_clip;
        return node;
    }

    static bool decode(const Node& node, k2::Camera::OrthographicTraits& orthographic_traits) {
        orthographic_traits.left = node["Left"].as<float>();
        orthographic_traits.right = node["Right"].as<float>();
        orthographic_traits.top = node["Top"].as<float>();
        orthographic_traits.bottom = node["Bottom"].as<float>();
        orthographic_traits.far_clip = node["FarClip"].as<float>();
        orthographic_traits.near_clip = node["NearClip"].as<float>();
        return true;
    }
};

template <> struct convert<k2::Camera::PerspectiveTraits> {
    static Node encode(const k2::Camera::PerspectiveTraits& perspective_traits) {
        YAML::Node node;
        node["FOV"] = perspective_traits.fov;
        node["AspectRatio"] = perspective_traits.aspect_ratio;
        node["FarClip"] = perspective_traits.far_clip;
        node["NearClip"] = perspective_traits.near_clip;
        return node;
    }

    static bool decode(const Node& node, k2::Camera::PerspectiveTraits& perspective_traits) {
        perspective_traits.fov = node["FOV"].as<float>();
        perspective_traits.aspect_ratio = node["AspectRatio"].as<float>();
        perspective_traits.far_clip = node["FarClip"].as<float>();
        perspective_traits.near_clip = node["NearClip"].as<float>();
        return true;
    }
};

template <> struct convert<k2::Camera> {
    static Node encode(const k2::Camera& camera) {
        YAML::Node node;
        node["Position"] = camera.position;
        node["Target"] = camera.target;
        node["Up"] = camera.up;
        std::visit(
            [&](auto&& traits) {
                if constexpr (std::is_same_v<std::decay_t<decltype(traits)>, k2::Camera::OrthographicTraits>) {
                    node["ProjectionType"] = "Orthographic";
                    node["ProjectionTraits"] = traits;
                } else if constexpr (std::is_same_v<std::decay_t<decltype(traits)>, k2::Camera::PerspectiveTraits>) {
                    node["ProjectionType"] = "Perspective";
                    node["ProjectionTraits"] = traits;
                }
            },
            camera.projection_traits);
        return node;
    }

    static bool decode(const Node& node, k2::Camera& camera) {
        camera.position = node["Position"].as<glm::vec3>();
        camera.target = node["Target"].as<glm::vec3>();
        camera.up = node["Up"].as<glm::vec3>();

        const auto& projection_type = node["ProjectionType"].as<std::string>();
        if (projection_type == "Orthographic") {
            camera.projection_traits = node["ProjectionTraits"].as<k2::Camera::OrthographicTraits>();
        } else if (projection_type == "Perspective") {
            camera.projection_traits = node["ProjectionTraits"].as<k2::Camera::PerspectiveTraits>();
        }

        return true;
    }
};

}