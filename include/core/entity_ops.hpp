#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <entt/entt.hpp>

namespace k2 {

entt::entity create_entity(entt::registry& registry);

// Copies all components except RelationComponent and ScriptComponent;
// LuaComponent data is deep-copied.
entt::entity clone_entity(entt::registry& registry, entt::entity src);

void destroy_with_children(entt::registry& registry, entt::entity entity);

entt::entity find_by_tag(entt::registry& registry, std::string_view tag);

std::vector<entt::entity> find_all_by_tag(entt::registry& registry, std::string_view tag);

std::vector<entt::entity> find_with_components(entt::registry& registry, std::span<const std::string> names);

}
