#include "core/entity_ops.hpp"

#include <type_traits>

#include "components/animation.hpp"
#include "components/camera.hpp"
#include "components/light.hpp"
#include "components/sprite.hpp"
#include "components/tag.hpp"
#include "components/text.hpp"
#include "components/transform.hpp"

namespace k2 {
namespace {

    template <class Component> void copy_component(entt::registry& registry, entt::entity src, entt::entity dst) {
        if constexpr (std::is_empty_v<Component>) {
            if (registry.all_of<Component>(src)) {
                registry.emplace<Component>(dst);
            }
        } else if (const auto* component = registry.try_get<Component>(src)) {
            registry.emplace<Component>(dst, *component);
        }
    }

}

entt::entity clone_entity(entt::registry& registry, entt::entity src) {
    auto dst = registry.create();
    copy_component<TagComponent>(registry, src, dst);
    copy_component<TransformComponent>(registry, src, dst);
    copy_component<Camera>(registry, src, dst);
    copy_component<MainCamera>(registry, src, dst);
    copy_component<SpriteComponent>(registry, src, dst);
    copy_component<TextComponent>(registry, src, dst);
    copy_component<AnimationComponent>(registry, src, dst);
    copy_component<AmbientLight>(registry, src, dst);
    copy_component<PointLight>(registry, src, dst);
    copy_component<SpotLight>(registry, src, dst);
    copy_component<SpriteLight>(registry, src, dst);
    return dst;
}

entt::entity find_by_tag(entt::registry& registry, std::string_view tag) {
    for (auto [entity, tag_component] : registry.view<TagComponent>().each()) {
        if (tag_component.tag == tag) {
            return entity;
        }
    }
    return entt::null;
}

std::vector<entt::entity> find_all_by_tag(entt::registry& registry, std::string_view tag) {
    std::vector<entt::entity> matches;
    for (auto [entity, tag_component] : registry.view<TagComponent>().each()) {
        if (tag_component.tag == tag) {
            matches.push_back(entity);
        }
    }
    return matches;
}

}
