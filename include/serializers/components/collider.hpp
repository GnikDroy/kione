#pragma once
#include <string>

#include "components/collider.hpp"
#include "serializers/utils.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::ColliderComponent> {
    static Node encode(const k2::ColliderComponent& collider) {
        YAML::Node node;
        std::visit(
            [&](const auto& shape) {
                using Shape = std::decay_t<decltype(shape)>;
                if constexpr (std::is_same_v<Shape, k2::BoxShape>) {
                    node["Shape"] = "Box";
                    node["Size"] = shape.size;
                } else if constexpr (std::is_same_v<Shape, k2::CircleShape>) {
                    node["Shape"] = "Circle";
                    node["Radius"] = shape.radius;
                } else {
                    node["Shape"] = "Pill";
                    node["Radius"] = shape.radius;
                    node["HalfHeight"] = shape.half_height;
                }
            },
            collider.shape);
        if (collider.layer != 1) {
            node["Layer"] = collider.layer;
        }
        if (collider.mask != 0xffffffff) {
            node["Mask"] = collider.mask;
        }
        return node;
    }

    static bool decode(const Node& node, k2::ColliderComponent& collider) {
        if (!node.IsMap()) {
            return false;
        }
        const auto& shape = node["Shape"].as<std::string>();
        if (shape == "Box") {
            collider.shape = k2::BoxShape { .size = node["Size"].as<glm::vec2>() };
        } else if (shape == "Circle") {
            collider.shape = k2::CircleShape { .radius = node["Radius"].as<float>() };
        } else if (shape == "Pill") {
            collider.shape = k2::PillShape { .radius = node["Radius"].as<float>(),
                .half_height = node["HalfHeight"].as<float>() };
        } else {
            return false;
        }
        collider.layer = node["Layer"].as<std::uint32_t>(1);
        collider.mask = node["Mask"].as<std::uint32_t>(0xffffffff);
        return true;
    }
};
}
