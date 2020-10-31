#pragma once
#include "core/mouse.hpp"

namespace k2 {
class Window;
struct MouseButtonEvent {
    Window* window;
    MouseDevice::ButtonCode code;
    KeyboardDevice::KeyMod mods;
    KeyboardDevice::KeyState state;
};

struct MouseDropEvent {
    Window* window;
    std::vector<std::string> paths;
};

struct CursorPositionEvent {
    Window* window;
    double x, y;
};

struct CursorEnterExitEvent {
    Window* window;
    bool state;
};

struct ScrollEvent {
    Window* window;
    double x, y;
};
}  // namespace k2