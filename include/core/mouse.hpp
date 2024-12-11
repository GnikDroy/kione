#pragma once

#include "core/input_device.hpp"

namespace k2 {
class Window;

class MouseDevice : public IInputDevice {
public:
    enum class ButtonCode {
        Zero = 0,
        One = 1,
        Two = 2,
        Three = 3,
        Four = 4,
        Five = 5,
        Six = 6,
        Seven = 7,

        Last = Seven,
        Left = Zero,
        Right = One,
        Middle = Two
    };

    enum class ButtonState {
        press = 1,
        release = 0,
        repeat = 2,
        unknown = -1,
    };

    enum class CursorMode {
        Normal,
        Hidden,
        Disabled,
    };

    friend class Window;

    MouseDevice::ButtonState get_state(ButtonCode button);
    void set_cursor_mode(CursorMode mode);

    ~MouseDevice() = default;

private:
    explicit MouseDevice(Window*);
    Window* window_instance;
};
} // namespace k2
