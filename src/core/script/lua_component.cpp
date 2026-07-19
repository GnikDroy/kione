#include "core/script/lua_component.hpp"

#include <unordered_map>

namespace k2 {
namespace {

    sol::table copy_table(sol::state_view& lua, const sol::table& source,
        std::unordered_map<const void*, sol::table>& seen) {
        if (auto it = seen.find(source.pointer()); it != seen.end()) {
            return it->second;
        }
        sol::table copy = lua.create_table();
        seen.emplace(source.pointer(), copy);
        for (const auto& [key, value] : source.pairs()) {
            if (value.get_type() == sol::type::table) {
                copy.raw_set(key, copy_table(lua, value.as<sol::table>(), seen));
            } else {
                copy.raw_set(key, value);
            }
        }
        sol::object metatable = source[sol::metatable_key];
        if (metatable.is<sol::table>()) {
            copy[sol::metatable_key] = metatable;
        }
        return copy;
    }

}

sol::table& lua_component(sol::state& lua, entt::registry& registry, entt::entity entity) {
    auto& component = registry.get_or_emplace<LuaComponent>(entity);
    if (!component.valid()) {
        component = lua.create_table();
    }
    return component;
}

sol::table deep_copy_table(const sol::table& source) {
    sol::state_view lua { source.lua_state() };
    std::unordered_map<const void*, sol::table> seen;
    return copy_table(lua, source, seen);
}

}
