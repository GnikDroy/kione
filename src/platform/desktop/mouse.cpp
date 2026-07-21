#include <GLFW/glfw3.h>

#include "core/mouse.hpp"
#include "core/window.hpp"
#include "events/mouse.hpp"
#include "platform/desktop/window_impl.hpp"

namespace k2 {
MouseDevice::MouseDevice(Window* w)
    : window_instance(w) {
    glfwSetCursorPosCallback(w->impl->window.get(), [](auto glfw_window, auto x, auto y) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<CursorPositionEvent>();
        event->x = x;
        event->y = y;
        window->events.push(std::move(event));
    });

    glfwSetScrollCallback(w->impl->window.get(), [](auto glfw_window, auto x, auto y) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<ScrollEvent>();
        event->x = x;
        event->y = y;
        window->events.push(std::move(event));
    });

    glfwSetMouseButtonCallback(w->impl->window.get(), [](auto glfw_window, auto code, auto state, auto mods) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<MouseButtonEvent>();
        event->code = static_cast<k2::MouseDevice::ButtonCode>(code);
        event->state = static_cast<KeyboardDevice::KeyState>(state);
        event->mods = static_cast<KeyboardDevice::KeyMod>(mods);
        window->events.push(std::move(event));
    });

    glfwSetCursorEnterCallback(w->impl->window.get(), [](auto glfw_window, auto state) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<CursorEnterExitEvent>();
        event->state = static_cast<bool>(state);
        window->events.push(std::move(event));
    });

    glfwSetDropCallback(w->impl->window.get(), [](auto glfw_window, auto count, auto paths) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto dropped = std::span { paths, std::size_t(count) };
        auto event = std::make_unique<MouseDropEvent>();
        event->paths.assign(dropped.begin(), dropped.end());
        window->events.push(std::move(event));
    });
}

MouseDevice::ButtonState MouseDevice::get_state(ButtonCode button) {
    return static_cast<MouseDevice::ButtonState>(
        glfwGetMouseButton(window_instance->impl->window.get(), static_cast<int>(button)));
}

void MouseDevice::set_cursor_mode(CursorMode mode) {
    int glfw_mode;
    if (mode == CursorMode::Hidden)
        glfw_mode = GLFW_CURSOR_HIDDEN;
    else if (mode == CursorMode::Disabled)
        glfw_mode = GLFW_CURSOR_DISABLED;
    else
        glfw_mode = GLFW_CURSOR_NORMAL;
    glfwSetInputMode(window_instance->impl->window.get(), GLFW_CURSOR, glfw_mode);
}
} // namespace k2
