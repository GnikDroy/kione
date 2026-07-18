#pragma once

#include <string_view>

#include <sol/sol.hpp>

#include "core/script/lua_entity.hpp"

namespace k2 {

struct ScriptHost {
    virtual ~ScriptHost() = default;

    virtual sol::object find(std::string_view tag) = 0;
    virtual sol::table find_all(std::string_view tag) = 0;
    virtual LuaEntity spawn(std::string_view tag, float x, float y) = 0;
    virtual LuaEntity clone(const LuaEntity& source) = 0;
    virtual void destroy(const LuaEntity& target) = 0;
};

}
