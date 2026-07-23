#include "core/entity_ops.hpp"

#include <algorithm>
#include <format>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

#include "components/animation.hpp"
#include "components/audio.hpp"
#include "components/camera.hpp"
#include "components/collider.hpp"
#include "components/environment.hpp"
#include "components/light.hpp"
#include "components/relation.hpp"
#include "components/script.hpp"
#include "components/sprite.hpp"
#include "components/tag.hpp"
#include "components/text.hpp"
#include "components/tilemap.hpp"
#include "components/transform.hpp"
#include "core/script/lua_component.hpp"

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

entt::entity scene_root(entt::registry& registry) {
    for (auto [entity, relation] : registry.view<RelationComponent>().each()) {
        if (relation.parent == entt::null) {
            return entity;
        }
    }
    auto root = registry.create();
    registry.emplace<RelationComponent>(root);
    registry.emplace<TransformComponent>(root);
    return root;
}

entt::entity create_entity(entt::registry& registry) {
    auto entity = registry.create();
    registry.emplace<TransformComponent>(entity);
    RelationComponent::attach_last(registry, entity, scene_root(registry));
    return entity;
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
    copy_component<TileMapComponent>(registry, src, dst);
    copy_component<AudioSourceComponent>(registry, src, dst);
    copy_component<ColliderComponent>(registry, src, dst);
    copy_component<Environment>(registry, src, dst);
    copy_component<PointLight>(registry, src, dst);
    copy_component<SpotLight>(registry, src, dst);
    copy_component<SpriteLight>(registry, src, dst);
    if (const auto* data = registry.try_get<LuaComponent>(src); data != nullptr && data->valid()) {
        registry.emplace<LuaComponent>(dst, deep_copy_table(*data));
    }
    return dst;
}

void destroy_with_children(entt::registry& registry, entt::entity entity) {
    RelationComponent::detach(registry, entity);
    std::vector<entt::entity> doomed { entity };
    auto children = RelationComponent::get_children(registry, entity, true);
    std::ranges::transform(children, std::back_inserter(doomed), [](auto& child) { return child.first; });
    registry.destroy(doomed.begin(), doomed.end());
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

namespace {

    template <class Component> bool has_component(entt::registry& registry, entt::entity entity) {
        return registry.all_of<Component>(entity);
    }

    using ComponentCheck = bool (*)(entt::registry&, entt::entity);

    const std::unordered_map<std::string_view, ComponentCheck> component_checks {
        { "Tag", has_component<TagComponent> },
        { "Transform", has_component<TransformComponent> },
        { "Sprite", has_component<SpriteComponent> },
        { "Text", has_component<TextComponent> },
        { "Animation", has_component<AnimationComponent> },
        { "TileMap", has_component<TileMapComponent> },
        { "AudioSource", has_component<AudioSourceComponent> },
        { "Collider", has_component<ColliderComponent> },
        { "Environment", has_component<Environment> },
        { "PointLight", has_component<PointLight> },
        { "SpotLight", has_component<SpotLight> },
        { "SpriteLight", has_component<SpriteLight> },
        { "Camera", has_component<Camera> },
        { "MainCamera", has_component<MainCamera> },
        { "Script", has_component<ScriptComponent> },
        { "Data", has_component<LuaComponent> },
    };

}

std::vector<entt::entity> find_with_components(entt::registry& registry, std::span<const std::string> names) {
    std::vector<ComponentCheck> checks;
    for (const auto& name : names) {
        auto it = component_checks.find(name);
        if (it == component_checks.end()) {
            throw std::runtime_error(std::format("Unknown component name '{}'", name));
        }
        checks.push_back(it->second);
    }

    std::vector<entt::entity> matches;
    for (auto entity : registry.view<entt::entity>()) {
        if (std::ranges::all_of(checks, [&](auto check) { return check(registry, entity); })) {
            matches.push_back(entity);
        }
    }
    return matches;
}

}
