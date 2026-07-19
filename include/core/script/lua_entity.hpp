#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <entt/entt.hpp>

#include "components/animation.hpp"
#include "components/light.hpp"
#include "components/sprite.hpp"
#include "components/tag.hpp"
#include "components/text.hpp"
#include "components/transform.hpp"

namespace k2 {

// Handle to an entity that is safe against destruction and slot recycling.
struct LuaEntity {
    entt::entity entity { entt::null };
    entt::registry* registry {};
    std::shared_ptr<const std::uint64_t> epoch_token {};
    std::uint64_t stamp {};

    [[nodiscard]] bool valid() const {
        return epoch_token && *epoch_token == stamp && registry && registry->valid(entity);
    }

    [[nodiscard]] TransformComponent* transform() const {
        return valid() ? registry->try_get<TransformComponent>(entity) : nullptr;
    }
    [[nodiscard]] SpriteComponent* sprite() const {
        return valid() ? registry->try_get<SpriteComponent>(entity) : nullptr;
    }
    [[nodiscard]] TextComponent* text() const {
        return valid() ? registry->try_get<TextComponent>(entity) : nullptr;
    }
    [[nodiscard]] AnimationComponent* animation() const {
        return valid() ? registry->try_get<AnimationComponent>(entity) : nullptr;
    }
    [[nodiscard]] PointLight* point_light() const {
        return valid() ? registry->try_get<PointLight>(entity) : nullptr;
    }
    [[nodiscard]] SpotLight* spot_light() const {
        return valid() ? registry->try_get<SpotLight>(entity) : nullptr;
    }
    [[nodiscard]] AmbientLight* ambient_light() const {
        return valid() ? registry->try_get<AmbientLight>(entity) : nullptr;
    }
    [[nodiscard]] SpriteLight* sprite_light() const {
        return valid() ? registry->try_get<SpriteLight>(entity) : nullptr;
    }
    [[nodiscard]] std::string tag() const {
        if (!valid()) {
            return {};
        }
        auto* tag_component = registry->try_get<TagComponent>(entity);
        return tag_component ? tag_component->tag : std::string {};
    }

    [[nodiscard]] std::uint64_t id() const { return entt::to_integral(entity); }

    bool operator==(const LuaEntity& other) const {
        return registry == other.registry && entity == other.entity;
    }
};

}
