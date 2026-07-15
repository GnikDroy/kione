#pragma once

#include <string>

#include <entt/entt.hpp>

#include "components/animation.hpp"
#include "components/light.hpp"
#include "components/sprite.hpp"
#include "components/tag.hpp"
#include "components/transform.hpp"

namespace k2 {

struct LuaEntity {
    entt::entity entity { entt::null };
    entt::registry* registry {};

    [[nodiscard]] TransformComponent* transform() const { return registry->try_get<TransformComponent>(entity); }
    [[nodiscard]] SpriteComponent* sprite() const { return registry->try_get<SpriteComponent>(entity); }
    [[nodiscard]] AnimationComponent* animation() const { return registry->try_get<AnimationComponent>(entity); }
    [[nodiscard]] PointLight* point_light() const { return registry->try_get<PointLight>(entity); }
    [[nodiscard]] SpotLight* spot_light() const { return registry->try_get<SpotLight>(entity); }
    [[nodiscard]] AmbientLight* ambient_light() const { return registry->try_get<AmbientLight>(entity); }
    [[nodiscard]] SpriteLight* sprite_light() const { return registry->try_get<SpriteLight>(entity); }
    [[nodiscard]] std::string tag() const {
        auto* tag_component = registry->try_get<TagComponent>(entity);
        return tag_component ? tag_component->tag : std::string {};
    }
    [[nodiscard]] bool valid() const { return registry->valid(entity); }
};

}
