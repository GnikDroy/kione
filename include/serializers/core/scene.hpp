#pragma once

#include <queue>
#include <unordered_map>
#include <vector>

#include <yaml-cpp/yaml.h>

// Note that these headers need to be included
// So that yaml-cpp recognizes how to deserialize these components
#include "serializers/components/animation.hpp" // IWYU pragma: keep
#include "serializers/components/camera.hpp" // IWYU pragma: keep
#include "serializers/components/collider.hpp" // IWYU pragma: keep
#include "serializers/components/environment.hpp" // IWYU pragma: keep
#include "serializers/components/light.hpp" // IWYU pragma: keep
#include "serializers/components/relation.hpp" // IWYU pragma: keep
#include "serializers/components/script.hpp" // IWYU pragma: keep
#include "serializers/components/sprite.hpp" // IWYU pragma: keep
#include "serializers/components/tag.hpp" // IWYU pragma: keep
#include "serializers/components/audio.hpp" // IWYU pragma: keep
#include "serializers/components/text.hpp" // IWYU pragma: keep
#include "serializers/components/tilemap.hpp" // IWYU pragma: keep
#include "serializers/components/transform.hpp" // IWYU pragma: keep
#include "serializers/core/scene.hpp" // IWYU pragma: keep
#include "serializers/utils.hpp" // IWYU pragma: keep

#include "core/scene.hpp"

namespace YAML {
template <> struct convert<k2::Scene> {
    static Node encode(const k2::Scene& scene) {
        YAML::Node node { YAML::NodeType::Sequence };
        auto& registry = scene.registry;

        // Serialized IDs are stable. Root is 0, and indices go in BFS order.
        std::vector<entt::entity> order;
        std::unordered_map<entt::entity, entt::entity> label;
        std::queue<entt::entity> pending;
        registry.view<k2::RelationComponent>().each([&](auto entity, const k2::RelationComponent& relation) {
            if (relation.parent == entt::null) {
                pending.push(entity);
            }
        });
        while (!pending.empty()) {
            auto entity = pending.front();
            pending.pop();
            label.emplace(entity, static_cast<entt::entity>(order.size()));
            order.push_back(entity);
            if (const auto* relation = registry.try_get<k2::RelationComponent>(entity)) {
                auto curr = relation->first;
                for (std::size_t i {}; i < relation->children; i++) {
                    pending.push(curr);
                    curr = registry.get<k2::RelationComponent>(curr).next;
                }
            }
        }
        auto relabel = [&](entt::entity e) { return e == entt::null ? entt::null : label.at(e); };

        auto serialize = [&]<class Component>(YAML::Node& enode, const char* key, entt::entity entity) {
            if constexpr (std::is_empty_v<Component>) {
                if (registry.all_of<Component>(entity)) {
                    enode[key] = Component {};
                }
            } else {
                if (const auto* component = registry.try_get<Component>(entity)) {
                    enode[key] = *component;
                }
            }
        };

        for (auto entity : order) {
            YAML::Node entity_node;
            entity_node["Entity"] = relabel(entity);
            serialize.template operator()<k2::TagComponent>(entity_node, "TagComponent", entity);
            serialize.template operator()<k2::TransformComponent>(entity_node, "TransformComponent", entity);
            // RelationComponent stores entity references, so relabel.
            if (const auto* relation = registry.try_get<k2::RelationComponent>(entity)) {
                k2::RelationComponent relabeled = *relation;
                relabeled.parent = relabel(relabeled.parent);
                relabeled.first = relabel(relabeled.first);
                relabeled.next = relabel(relabeled.next);
                relabeled.prev = relabel(relabeled.prev);
                entity_node["RelationComponent"] = relabeled;
            }
            serialize.template operator()<k2::Camera>(entity_node, "Camera", entity);
            serialize.template operator()<k2::MainCamera>(entity_node, "MainCamera", entity);
            serialize.template operator()<k2::SpriteComponent>(entity_node, "SpriteComponent", entity);
            serialize.template operator()<k2::TextComponent>(entity_node, "TextComponent", entity);
            serialize.template operator()<k2::AudioSourceComponent>(entity_node, "AudioSourceComponent", entity);
            serialize.template operator()<k2::ColliderComponent>(entity_node, "ColliderComponent", entity);
            serialize.template operator()<k2::Environment>(entity_node, "Environment", entity);
            serialize.template operator()<k2::AnimationComponent>(entity_node, "AnimationComponent", entity);
            serialize.template operator()<k2::TileMapComponent>(entity_node, "TileMapComponent", entity);
            serialize.template operator()<k2::ScriptComponent>(entity_node, "ScriptComponent", entity);
            serialize.template operator()<k2::PointLight>(entity_node, "PointLight", entity);
            serialize.template operator()<k2::SpotLight>(entity_node, "SpotLight", entity);
            serialize.template operator()<k2::SpriteLight>(entity_node, "SpriteLight", entity);
            node.push_back(entity_node);
        }
        return node;
    }

    // Decoding lives in k2::SceneLoader: stored entity references must be
    // remapped, which convert<> cannot do.
};
}
