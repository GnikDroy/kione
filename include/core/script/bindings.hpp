#pragma once

#include <sol/sol.hpp>

namespace k2 {
class Window;
struct ScriptHost;

void bind_script_api(sol::state& lua, Window& window, const bool& input_enabled, ScriptHost& host);

void bind_constants(sol::state& lua);

}
