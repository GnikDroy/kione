#pragma once
#include <functional>

#include "core/input_device.hpp"
#include "core/keyboard.hpp"

namespace k2 {
class Window;

class MouseDevice : public IInputDevice {
   public:
    enum class ButtonCode {
        Button0 = 0,
        Button1 = 1,
        Button2 = 2,
        Button3 = 3,
        Button4 = 4,
        Button5 = 5,
        Button6 = 6,
        Button7 = 7,

        ButtonLast = Button7,
        ButtonLeft = Button0,
        ButtonRight = Button1,
        ButtonMiddle = Button2
    };

    enum class CursorMode {
        Normal,
        Hidden,
        Disabled, 
    };

    friend class Window;

    KeyboardDevice::KeyState get_state(ButtonCode button);
    void set_cursor_mode(CursorMode mode);

    ~MouseDevice();

   private:
    MouseDevice(Window*);
    Window* window_instance;
};
}  // namespace k2