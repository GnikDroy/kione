#pragma once

#include <optional>

#include <sol/sol.hpp>

namespace k2 {
struct Event;

struct TranslatedEvent {
    sol::table table;
    bool input = true;
};

std::optional<TranslatedEvent> translate_event(sol::state& lua, const Event* event);

}
