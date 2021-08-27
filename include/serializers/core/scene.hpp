#pragma once
#include "serializers/components/camera.hpp"
#include "serializers/components/relation.hpp"
#include "serializers/components/sprite.hpp"
#include "serializers/components/transform.hpp"
#include "serializers/core/scene.hpp"
#include <cereal/cereal.hpp>

namespace cereal {
template <class Archive> void save(Archive& ar, const k2::Scene& scene) {
    auto& registry = scene.registry;
    entt::snapshot { registry }
        .entities(ar)
        .component<k2::TransformComponent, k2::SpriteComponent, k2::RelationComponent, k2::Camera>(ar);
}

template <class Archive> void load(Archive& ar, k2::Scene& scene) {
    auto& registry = scene.registry;
    entt::snapshot_loader { registry }
        .entities(ar)
        .component<k2::TransformComponent, k2::SpriteComponent, k2::RelationComponent, k2::Camera>(ar)
        .orphans();
}
}