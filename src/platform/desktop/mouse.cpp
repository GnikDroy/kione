#include "core/mouse.hpp"

#include "GLFW/glfw3.h"
#include "core/window.hpp"
#include "events/mouse.hpp"
#include "platform/desktop/window_impl.hpp"

namespace k2 {
    MouseDevice::MouseDevice(Window *w) : window_instance(w) {
        glfwSetCursorPosCallback(w->impl->window.get(), [](auto glfw_window, auto x, auto y) {
            auto window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
            CursorPositionEvent event;
            event.x = x;
            event.y = y;
            window->impl->event_handler(event);
        });

        glfwSetScrollCallback(w->impl->window.get(), [](auto glfw_window, auto x, auto y) {
            auto window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
            ScrollEvent event;
            event.x = x;
            event.y = y;
            window->impl->event_handler(event);
        });

        glfwSetMouseButtonCallback(w->impl->window.get(), [](auto glfw_window, auto code, auto mod, auto state) {
            auto window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
            MouseButtonEvent event;
            event.code = static_cast<k2::MouseDevice::ButtonCode>(code);
            event.mods = static_cast<KeyboardDevice::KeyMod>(mod);
            event.state = static_cast<KeyboardDevice::KeyState>(state);
            window->impl->event_handler(event);
        });

        glfwSetCursorEnterCallback(w->impl->window.get(), [](auto glfw_window, auto state) {
            auto window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
            CursorEnterExitEvent event;
            event.state = static_cast<bool>(state);
            window->impl->event_handler(event);
        });

        glfwSetDropCallback(w->impl->window.get(), [](auto glfw_window, auto count, auto paths) {
            auto window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
            std::vector<std::string> paths_vec;
            for (int i{}; i < count; i++) { paths_vec.emplace_back(paths[i]); }
            MouseDropEvent event;
            event.paths = std::move(paths_vec);
            window->impl->event_handler(event);
        });
    }

    MouseDevice::~MouseDevice() {}

    MouseDevice::ButtonState MouseDevice::get_state(ButtonCode button) {
        return static_cast<MouseDevice::ButtonState>(glfwGetKey(
                window_instance->impl->window.get(), static_cast<int>(button)));
    }

    void MouseDevice::set_cursor_mode(CursorMode mode) {
        int glfw_mode{};
        if (mode == CursorMode::Hidden)
            glfw_mode = GLFW_CURSOR_HIDDEN;
        else if (mode == CursorMode::Disabled)
            glfw_mode = GLFW_CURSOR_DISABLED;
        else
            glfw_mode = GLFW_CURSOR_NORMAL;
        glfwSetInputMode(window_instance->impl->window.get(), GLFW_CURSOR,
                         glfw_mode);
    }
}  // namespace k2