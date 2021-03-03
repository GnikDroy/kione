#include "core/keyboard.hpp"

#include "GLFW/glfw3.h"
#include "core/window.hpp"
#include "events/keyboard.hpp"
#include "platform/desktop/window_impl.hpp"

namespace k2 {

    KeyboardDevice::KeyboardDevice(Window *w) : window_instance(w) {
        glfwSetKeyCallback(
                w->impl->window.get(), [](GLFWwindow *glfw_window, int key,
                                          int scan_code, int action, int mod) {
                    auto window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
                    KeyboardKeyEvent event;
                    event.code = static_cast<KeyboardDevice::KeyCode>(key);
                    event.scan_code = scan_code;
                    event.state = static_cast<KeyboardDevice::KeyState>(action);
                    event.mods = static_cast<KeyboardDevice::KeyMod>(mod);
                    window->impl->event_handler(event);
                });
        glfwSetCharCallback(w->impl->window.get(), [](GLFWwindow *glfw_window, unsigned int code_point) {
            auto window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
            KeyboardCharEvent event;
            event.code = code_point;
            window->impl->event_handler(event);
        });
    }

    KeyboardDevice::~KeyboardDevice() {}

// This will never return REPEAT or UNKNOWN
    KeyboardDevice::KeyState KeyboardDevice::get_state(
            KeyboardDevice::KeyCode code) const {
        return static_cast<KeyboardDevice::KeyState>(glfwGetKey(
                window_instance->impl->window.get(), static_cast<int>(code)));
    }
}  // namespace k2