#pragma once

#include "core/keyboard.hpp"
#include "events/event.hpp"

namespace k2 {
    using namespace k2::literals;
    struct KeyboardKeyEvent : public Event {
        KeyboardKeyEvent(): Event("KeyboardKeyEvent"_fnv1a) {}
        KeyboardDevice::KeyCode code{};
        int scan_code{};
        KeyboardDevice::KeyState state{};
        KeyboardDevice::KeyMod mods{};
    };

    struct KeyboardCharEvent : public Event {
        KeyboardCharEvent() : Event("KeyboardCharEvent"_fnv1a) {}
        unsigned int code{};
    };
}  // namespace k2