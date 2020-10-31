#pragma once
#include "core/keyboard.hpp"

namespace k2 {
    class Window;
struct KeyboardKeyEvent {
    Window* window;
    KeyboardDevice::KeyCode code;
    int scan_code;
    KeyboardDevice::KeyState state;
    KeyboardDevice::KeyMod mods;
};

struct KeyboardCharEvent {
    Window* window;
    unsigned int code;
};
}  // namespace k2