#include "platform/desktop/window_impl.hpp"
#include "events/window.hpp"
#include <glad/glad.h>

namespace k2 {
Window::Impl::Impl(Window* win, const WindowConfig& config) {
    glfw_data = {
        .title = config.title,
        .vsync = true,
    };

    glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

    window.reset(glfwCreateWindow((int)config.width, (int)config.height, glfw_data.title.c_str(), nullptr, nullptr));

    if (!window) {
        k2::Log::core().critical("Failed to create GLFW window");
        glfwTerminate();
    } else {
        k2::Log::core().info("Created GLFW window");
        glfw_window_count++;
    }

    glfwSetErrorCallback(
        [](auto error_code, auto msg) { k2::Log::core().error(fmt::format("GLFW Error {} : {}", error_code, msg)); });

    glfwSetWindowUserPointer(window.get(), win);

    glfwMakeContextCurrent(window.get());
    glfwSwapInterval(glfw_data.vsync);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        k2::Log::core().critical("Failed to initialize GLAD.");
    } else {
        k2::Log::core().info("GLAD initialization successful.");
    }

    k2::Log::core().info(fmt::format("OpenGL version is {}", glGetString(GL_VERSION)));
    glViewport(0, 0, static_cast<int32_t>(config.width), static_cast<int32_t>(config.height));
    glEnable(GL_MULTISAMPLE);

    glfwSetWindowCloseCallback(window.get(), [](auto glfw_window) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<WindowCloseEvent>();
        window->events.push(std::move(event));
    });

    glfwSetWindowSizeCallback(window.get(), [](auto glfw_window, auto width, auto height) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<WindowResizeEvent>();
        event->width = width;
        event->height = height;
        window->events.push(std::move(event));
    });

    glfwSetFramebufferSizeCallback(window.get(), [](auto glfw_window, auto width, auto height) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<WindowFramebufferResizeEvent>();
        event->width = width;
        event->height = height;
        window->events.push(std::move(event));
    });

    glfwSetWindowContentScaleCallback(window.get(), [](auto glfw_window, auto x, auto y) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<WindowContentScaleChangeEvent>();
        event->x = x;
        event->y = y;
        window->events.push(std::move(event));
    });

    glfwSetWindowPosCallback(window.get(), [](auto glfw_window, auto x, auto y) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<WindowRepositionEvent>();
        event->x = x;
        event->y = y;
        window->events.push(std::move(event));
    });

    glfwSetWindowFocusCallback(window.get(), [](auto glfw_window, int focus) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<WindowFocusChangeEvent>();
        event->focused = static_cast<bool>(focus);
        window->events.push(std::move(event));
    });

    glfwSetWindowIconifyCallback(window.get(), [](auto glfw_window, int iconified) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<WindowIconifyEvent>();
        event->iconified = static_cast<bool>(iconified);
        window->events.push(std::move(event));
    });

    glfwSetWindowMaximizeCallback(window.get(), [](auto glfw_window, int maximized) {
        auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfw_window));
        auto event = std::make_unique<WindowMaximizeEvent>();
        event->maximized = static_cast<bool>(maximized);
        window->events.push(std::move(event));
    });
}

Window::Impl::~Impl() { glfw_window_count--; }
} // namespace k2
