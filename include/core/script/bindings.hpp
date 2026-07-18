#pragma once

#include <string>
#include <unordered_set>

#include <sol/sol.hpp>

namespace k2 {
class Window;
struct ScriptHost;

void bind_script_api(sol::state& lua, Window& window, const bool& input_enabled, ScriptHost& host);

void bind_constants(sol::state& lua);

std::unordered_set<std::string> table_string_keys(const sol::table& table);

void reset_table_to_baseline(sol::table table, const std::unordered_set<std::string>& baseline);

}
