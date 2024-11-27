#pragma once
#include "serializers/components/camera.hpp"
#include "serializers/components/relation.hpp"
#include "serializers/components/sprite.hpp"
#include "serializers/components/tag.hpp"
#include "serializers/components/transform.hpp"
#include "serializers/core/scene.hpp"
#include "serializers/utils.hpp"
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<k2::Scene> {
    static Node encode(const k2::Scene& scene) {
        YAML::Node node;
        auto& registry = scene.registry;

        auto serialize = [&]<class Component>(auto& node, const auto& label, const auto& entity) {
            if (const auto* component = registry.try_get<Component>(entity)) {
                node[label] = *component;
            }
        };

        for (auto entity : registry.view<entt::entity>()) {
            YAML::Node entity_node;
            entity_node["Entity"] = entity;
            serialize.template operator()<k2::TagComponent>(entity_node, "TagComponent", entity);
            serialize.template operator()<k2::TransformComponent>(entity_node, "TransformComponent", entity);
            serialize.template operator()<k2::RelationComponent>(entity_node, "RelationComponent", entity);
            serialize.template operator()<k2::Camera>(entity_node, "Camera", entity);
            serialize.template operator()<k2::SpriteComponent>(entity_node, "SpriteComponent", entity);
            node.push_back(entity_node);
        }
        return node;
    }

    static bool decode(const Node& node, k2::Scene& scene) {
        auto& registry = scene.registry;

        auto deserialize = [&]<class Component>(auto& node, const auto& label, const auto& entity) {
            if (node[label].IsDefined()) {
                auto component = node[label].as<Component>();
                registry.emplace<Component>(entity, component);
            }
        };

        for (auto entity_node : node) {
            auto entity = entity_node["Entity"].as<entt::entity>();
            entity = registry.create(entity);

            deserialize.template operator()<k2::TagComponent>(entity_node, "TagComponent", entity);
            deserialize.template operator()<k2::TransformComponent>(entity_node, "TransformComponent", entity);
            deserialize.template operator()<k2::RelationComponent>(entity_node, "RelationComponent", entity);
            deserialize.template operator()<k2::Camera>(entity_node, "Camera", entity);
            deserialize.template operator()<k2::SpriteComponent>(entity_node, "SpriteComponent", entity);
        }

        return true;
    }
};
}