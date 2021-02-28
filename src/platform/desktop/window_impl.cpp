// #include "glad/glad.h"
// glad before glfw
#include "core/event_dispatcher.hpp"
#include "core/logger.hpp"
#include "events/window.hpp"
#include "platform/desktop/window_impl.hpp"

namespace k2 {
    Window::Impl::Impl(Window *win, const WindowConfig &config) {
        glfw_data = {
                .title = config.title,
                .vsync = true,
        };
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window.reset(glfwCreateWindow(config.width, config.height,
                                      glfw_data.title.c_str(), nullptr, nullptr));

        if (!window) {
            k2::Logger::core->critical("Failed to create GLFW window");
            glfwTerminate();
        } else {
            k2::Logger::core->info("Created GLFW window");
            glfw_window_count++;
        }

        glfwSetErrorCallback([](auto error_code, auto msg) {
            k2::Logger::core->error(
                    fmt::format("GLFW Error {} : {}", error_code, msg));
        });

        glfwSetWindowUserPointer(window.get(), win);

        glfwSetWindowCloseCallback(window.get(), [](auto glfw_window) {
            event_dispatcher.enqueue<WindowCloseEvent>(WindowCloseEvent{
                    .window{reinterpret_cast<Window *>(
                                    glfwGetWindowUserPointer(glfw_window))},
            });
        });

        glfwSetWindowSizeCallback(window.get(), [](auto glfw_window, auto width,
                                                   auto height) {
            auto window =
                    reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
            event_dispatcher.enqueue<WindowResizeEvent>(WindowResizeEvent{
                    .window{reinterpret_cast<Window *>(window)},
                    .width{width},
                    .height{height},
            });
        });

        glfwSetWindowContentScaleCallback(
                window.get(), [](auto glfw_window, auto x, auto y) {
                    event_dispatcher.enqueue<WindowContentScaleChangeEvent>(
                            WindowContentScaleChangeEvent{
                                    .window{reinterpret_cast<Window *>(
                                                    glfwGetWindowUserPointer(glfw_window))},
                                    .x{x},
                                    .y{y},
                            });
                });

        glfwSetWindowPosCallback(window.get(), [](auto glfw_window, auto x,
                                                  auto y) {
            auto window =
                    reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw_window));
            event_dispatcher.enqueue<WindowRepositionEvent>(WindowRepositionEvent{
                    .window{window},
                    .x{x},
                    .y{y},
            });
        });

        glfwSetWindowFocusCallback(window.get(), [](auto glfw_window, int focus) {
            event_dispatcher.enqueue<WindowFocusChangeEvent>(WindowFocusChangeEvent{
                    .window{reinterpret_cast<Window *>(
                                    glfwGetWindowUserPointer(glfw_window))},
                    .focused{static_cast<bool>(focus)},
            });
        });

        glfwSetWindowIconifyCallback(
                window.get(), [](auto glfw_window, int iconified) {
                    event_dispatcher.enqueue<WindowIconifyEvent>(WindowIconifyEvent{
                            .window{reinterpret_cast<Window *>(
                                            glfwGetWindowUserPointer(glfw_window))},
                            .iconified{static_cast<bool>(iconified)},
                    });
                });

        glfwSetWindowMaximizeCallback(
                window.get(), [](auto glfw_window, int maximized) {
                    event_dispatcher.enqueue<WindowMaximizeEvent>(WindowMaximizeEvent{
                            .window{reinterpret_cast<Window *>(
                                            glfwGetWindowUserPointer(glfw_window))},
                            .maximized{static_cast<bool>(maximized)},
                    });
                });
    }

    Window::Impl::~Impl() { glfw_window_count--; }
}  // namespace k2
