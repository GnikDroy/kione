#pragma once

#include <sol/sol.hpp>

#include "core/ecs.hpp" // IWYU pragma: keep

namespace k2 {

using LuaComponent = sol::table;

sol::table& lua_component(sol::state& lua, entt::registry& registry, entt::entity entity);

sol::table deep_copy_table(const sol::table& source);

}
