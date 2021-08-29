#pragma once

#include "core/keyboard.hpp"
#include "events/event.hpp"

namespace k2 {
using namespace k2::literals;
struct KeyboardKeyEvent : public Event {
    constexpr static inline std::uint64_t hash = "KeyboardKeyEvent"_fnv1a;
    KeyboardKeyEvent()
        : Event(hash) { }
    KeyboardDevice::KeyCode code {};
    int scan_code {};
    KeyboardDevice::KeyState state {};
    KeyboardDevice::KeyMod mods {};
};

struct KeyboardCharEvent : public Event {
    constexpr static inline std::uint64_t hash = "KeyboardCharEvent"_fnv1a;
    KeyboardCharEvent()
        : Event(hash) { }
    unsigned int code {};
};
} // namespace k2