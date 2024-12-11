#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#elif __linux__
#define GLFW_EXPOSE_NATIVE_X11
#elif __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "core/window.hpp"
#include "events/event.hpp" // IWYU pragma: export
#include "platform/desktop/window_impl.hpp"

namespace k2 {

Window::Window(const WindowConfig& config)
    : impl(std::make_unique<Window::Impl>(this, config))
    , keyboard(this)
    , mouse(this) { }

Window::~Window() = default;

void Window::update() {
    glfwSwapBuffers(impl->window.get());
    glfwPollEvents();
}

std::uint32_t Window::get_width() const {
    int width, height;
    glfwGetWindowSize(impl->window.get(), &width, &height);
    return width;
}

std::uint32_t Window::get_height() const {
    int width, height;
    glfwGetWindowSize(impl->window.get(), &width, &height);
    return height;
}

void Window::set_vsync(bool status) {
    glfwSwapInterval(status);
    impl->glfw_data.vsync = status;
}

void* Window::get_native_handle() const {
#ifdef __linux__
    return reinterpret_cast<void*>(glfwGetX11Window(impl->window.get()));
#elif _WIN32
    return reinterpret_cast<void*>(glfwGetWin32Window(impl->window.get()));
#else
#error "Native window not implemented."
#endif
}

void* Window::get_native_display() const {
#ifdef __linux__
    return reinterpret_cast<void*>(glfwGetX11Display());
#elif _WIN32
    return nullptr;
#else
#error "Native display not implemented."
#endif
}

bool Window::is_vsync() const { return impl->glfw_data.vsync; }

} // namespace k2
