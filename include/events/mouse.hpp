#pragma once

#include "core/mouse.hpp"
#include "events/event.hpp"

namespace k2 {
using namespace k2::literals;
struct MouseButtonEvent : public Event {
    constexpr static inline std::uint64_t hash = "MouseButtonEvent"_fnv1a;
    MouseButtonEvent()
        : Event(hash) { }
    MouseDevice::ButtonCode code {};
    KeyboardDevice::KeyState state {};
    KeyboardDevice::KeyMod mods {};
};

struct MouseDropEvent : public Event {
    constexpr static inline std::uint64_t hash = "MouseDropEvent"_fnv1a;
    MouseDropEvent()
        : Event(hash) { }
    std::vector<std::string> paths;
};

struct CursorPositionEvent : public Event {
    constexpr static inline std::uint64_t hash = "CursorPositionEvent"_fnv1a;
    CursorPositionEvent()
        : Event(hash) { }
    double x {}, y {};
};

struct CursorEnterExitEvent : public Event {
    constexpr static inline std::uint64_t hash = "CursorEnterExitEvent"_fnv1a;
    CursorEnterExitEvent()
        : Event(hash) { }
    bool state {};
};

struct ScrollEvent : public Event {
    constexpr static inline std::uint64_t hash = "ScrollEvent"_fnv1a;
    ScrollEvent()
        : Event(hash) { }
    double x {}, y {};
};
} // namespace k2