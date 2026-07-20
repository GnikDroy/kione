#include "core/entity_ops.hpp"

#include <format>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

#include "components/animation.hpp"
#include "components/audio.hpp"
#include "components/camera.hpp"
#include "components/collider.hpp"
#include "components/light.hpp"
#include "components/script.hpp"
#include "components/sprite.hpp"
#include "components/tag.hpp"
#include "components/text.hpp"
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

entt::entity clone_entity(entt::registry& registry, entt::entity src) {
    auto dst = registry.create();
    copy_component<TagComponent>(registry, src, dst);
    copy_component<TransformComponent>(registry, src, dst);
    copy_component<Camera>(registry, src, dst);
    copy_component<MainCamera>(registry, src, dst);
    copy_component<SpriteComponent>(registry, src, dst);
    copy_component<TextComponent>(registry, src, dst);
    copy_component<AnimationComponent>(registry, src, dst);
    copy_component<AudioSourceComponent>(registry, src, dst);
    copy_component<ColliderComponent>(registry, src, dst);
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
        { "AudioSource", has_component<AudioSourceComponent> },
        { "Collider", has_component<ColliderComponent> },
        { "PointLight", has_component<PointLight> },
        { "SpotLight", has_component<SpotLight> },
        { "AmbientLight", has_component<AmbientLight> },
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
        bool all = true;
        for (auto check : checks) {
            all = all && check(registry, entity);
        }
        if (all) {
            matches.push_back(entity);
        }
    }
    return matches;
}

}
