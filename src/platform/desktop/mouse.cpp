#include "core/mouse.hpp"

#include "GLFW/glfw3.h"
#include "core/event_dispatcher.hpp"
#include "core/mouse.hpp"
#include "core/window.hpp"
#include "events/mouse.hpp"
#include "platform/desktop/window_impl.hpp"

namespace k2 {
MouseDevice::MouseDevice(Window* w) : window_instance(w) {
    glfwSetCursorPosCallback(
        w->impl->window.get(), [](auto glfw_window, auto x, auto y) {
            event_dispatcher.enqueue<CursorPositionEvent>(CursorPositionEvent{
                .window{reinterpret_cast<Window*>(
                    glfwGetWindowUserPointer(glfw_window))},
                .x{x},
                .y{y},
            });
        });

    glfwSetScrollCallback(w->impl->window.get(),
                          [](auto glfw_window, auto x, auto y) {
                              event_dispatcher.enqueue<ScrollEvent>(ScrollEvent{
                                  .window{reinterpret_cast<Window*>(
                                      glfwGetWindowUserPointer(glfw_window))},
                                  .x{x},
                                  .y{y},
                              });
                          });

    glfwSetMouseButtonCallback(
        w->impl->window.get(),
        [](auto glfw_window, auto code, auto mod, auto state) {
            event_dispatcher.enqueue<MouseButtonEvent>(MouseButtonEvent{
                .window{reinterpret_cast<Window*>(
                    glfwGetWindowUserPointer(glfw_window))},
                .code{static_cast<k2::MouseDevice::ButtonCode>(code)},
                .mods{static_cast<KeyboardDevice::KeyMod>(mod)},
                .state{static_cast<KeyboardDevice::KeyState>(state)},
            });
        });

    glfwSetCursorEnterCallback(
        w->impl->window.get(), [](auto glfw_window, auto state) {
            event_dispatcher.enqueue<CursorEnterExitEvent>(CursorEnterExitEvent{
                .window{reinterpret_cast<Window*>(
                    glfwGetWindowUserPointer(glfw_window))},
                .state{static_cast<bool>(state)},
            });
        });

    glfwSetDropCallback(w->impl->window.get(), [](auto glfw_window, auto count,
                                                  auto paths) {
        std::vector<std::string> paths_vec;
        for (int i{}; i < count; i++) {
            paths_vec.emplace_back(paths[i]);
        }
        event_dispatcher.enqueue<MouseDropEvent>(MouseDropEvent{
            .window{reinterpret_cast<Window*>(
                glfwGetWindowUserPointer(glfw_window))},
            .paths{std::move(paths_vec)},
        });
    });
}

MouseDevice::~MouseDevice() {}

KeyboardDevice::KeyState MouseDevice::get_state(ButtonCode button) {
    return static_cast<KeyboardDevice::KeyState>(glfwGetKey(
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