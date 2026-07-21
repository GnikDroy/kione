#pragma once

#include <cstdint>
#include <string>

#include <entt/entt.hpp>

#include "components/animation.hpp"
#include "components/audio.hpp"
#include "components/camera.hpp"
#include "components/collider.hpp"
#include "components/environment.hpp"
#include "components/light.hpp"
#include "components/sprite.hpp"
#include "components/tag.hpp"
#include "components/text.hpp"
#include "components/tilemap.hpp"
#include "components/transform.hpp"

namespace k2 {

struct LuaEntity {
    entt::entity entity { entt::null };
    entt::registry* registry {};

    [[nodiscard]] bool valid() const { return registry != nullptr && registry->valid(entity); }

    [[nodiscard]] TransformComponent* transform() const {
        return valid() ? registry->try_get<TransformComponent>(entity) : nullptr;
    }
    [[nodiscard]] SpriteComponent* sprite() const {
        return valid() ? registry->try_get<SpriteComponent>(entity) : nullptr;
    }
    [[nodiscard]] TextComponent* text() const { return valid() ? registry->try_get<TextComponent>(entity) : nullptr; }
    [[nodiscard]] AnimationComponent* animation() const {
        return valid() ? registry->try_get<AnimationComponent>(entity) : nullptr;
    }
    [[nodiscard]] TileMapComponent* tilemap() const {
        return valid() ? registry->try_get<TileMapComponent>(entity) : nullptr;
    }
    [[nodiscard]] PointLight* point_light() const { return valid() ? registry->try_get<PointLight>(entity) : nullptr; }
    [[nodiscard]] SpotLight* spot_light() const { return valid() ? registry->try_get<SpotLight>(entity) : nullptr; }

    [[nodiscard]] SpriteLight* sprite_light() const {
        return valid() ? registry->try_get<SpriteLight>(entity) : nullptr;
    }
    [[nodiscard]] Camera* camera() const { return valid() ? registry->try_get<Camera>(entity) : nullptr; }
    [[nodiscard]] AudioSourceComponent* audio_source() const {
        return valid() ? registry->try_get<AudioSourceComponent>(entity) : nullptr;
    }
    [[nodiscard]] ColliderComponent* collider() const {
        return valid() ? registry->try_get<ColliderComponent>(entity) : nullptr;
    }
    [[nodiscard]] Environment* environment() const {
        return valid() ? registry->try_get<Environment>(entity) : nullptr;
    }
    [[nodiscard]] std::string tag() const {
        if (!valid()) {
            return {};
        }
        auto* tag_component = registry->try_get<TagComponent>(entity);
        return tag_component ? tag_component->tag : std::string {};
    }
    void set_tag(const std::string& value) const {
        if (valid()) {
            registry->get_or_emplace<TagComponent>(entity).tag = value;
        }
    }

    [[nodiscard]] std::uint64_t id() const { return entt::to_integral(entity); }

    bool operator==(const LuaEntity& other) const { return registry == other.registry && entity == other.entity; }
};

}
