#pragma once

#include "core/mouse.hpp"
#include "events/event.hpp"

namespace k2 {
using namespace k2::literals;
struct MouseButtonEvent : public Event {
    MouseButtonEvent()
        : Event("MouseButtonEvent"_fnv1a) { }
    MouseDevice::ButtonCode code {};
    KeyboardDevice::KeyState state {};
    KeyboardDevice::KeyMod mods {};
};

struct MouseDropEvent : public Event {
    MouseDropEvent()
        : Event("MouseDropEvent"_fnv1a) { }
    std::vector<std::string> paths;
};

struct CursorPositionEvent : public Event {
    CursorPositionEvent()
        : Event("CursorPositionEvent"_fnv1a) { }
    double x {}, y {};
};

struct CursorEnterExitEvent : public Event {
    CursorEnterExitEvent()
        : Event("CursorEnterExitEvent"_fnv1a) { }
    bool state {};
};

struct ScrollEvent : public Event {
    ScrollEvent()
        : Event("ScrollEvent"_fnv1a) { }
    double x {}, y {};
};
} // namespace k2