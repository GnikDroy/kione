#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <entt/entt.hpp>

namespace k2 {

// Copies all components except RelationComponent and ScriptComponent
entt::entity clone_entity(entt::registry& registry, entt::entity src);

entt::entity find_by_tag(entt::registry& registry, std::string_view tag);

std::vector<entt::entity> find_all_by_tag(entt::registry& registry, std::string_view tag);

std::vector<entt::entity> find_with_components(entt::registry& registry, std::span<const std::string> names);

}
