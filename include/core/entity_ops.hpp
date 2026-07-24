#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <entt/entt.hpp>

namespace k2 {

entt::entity scene_root(entt::registry& registry);

entt::entity create_entity(entt::registry& registry);

// Children are cloned recursively, root is left detached.
// Copies all components except ScriptComponent (script host defers);
// LuaComponent data is deep-copied.
entt::entity clone_entity(entt::registry& registry, entt::entity src);

void destroy_with_children(entt::registry& registry, entt::entity entity);

entt::entity find_by_tag(entt::registry& registry, std::string_view tag);

std::vector<entt::entity> find_all_by_tag(entt::registry& registry, std::string_view tag);

std::vector<entt::entity> find_with_components(entt::registry& registry, std::span<const std::string> names);

}
