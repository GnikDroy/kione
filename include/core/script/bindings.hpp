#pragma once

#include <sol/sol.hpp>

namespace k2 {
class Window;

void bind_script_api(sol::state& lua, Window& window, const bool& input_enabled);

}
