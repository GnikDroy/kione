#pragma once
#include "serializers/components/camera.hpp"
#include "serializers/components/relation.hpp"
#include "serializers/components/sprite.hpp"
#include "serializers/components/tag.hpp"
#include "serializers/components/transform.hpp"
#include "serializers/core/scene.hpp"
#include <cereal/cereal.hpp>

namespace cereal {
template <class Archive> void save(Archive& ar, const k2::Scene& scene) {
    auto& registry = scene.registry;
    entt::snapshot { registry }
        .get<entt::entity>(ar)
        .get<k2::TransformComponent>(ar)
        .get<k2::SpriteComponent>(ar)
        .get<k2::RelationComponent>(ar)
        .get<k2::Camera>(ar)
        .get<k2::TagComponent>(ar);
}

template <class Archive> void load(Archive& ar, k2::Scene& scene) {
    auto& registry = scene.registry;
    entt::snapshot_loader { registry }
        .get<entt::entity>(ar)
        .get<k2::TransformComponent>(ar)
        .get<k2::SpriteComponent>(ar)
        .get<k2::RelationComponent>(ar)
        .get<k2::Camera>(ar)
        .get<k2::TagComponent>(ar)
        .orphans();
}
}
