
#include "glad/glad.h"

// Glad must be included before glfw
#include "GLFW/glfw3.h"
#include "core/window.hpp"
#include "platform/desktop/window_impl.hpp"

namespace k2 {

Window::Window(const WindowConfig &config)
    : impl(std::move(std::make_unique<Window::Impl>(this, config))), keyboard(this), mouse(this) {}

Window::~Window() {}

void Window::update() {
    glClearColor(1, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();
    glfwSwapBuffers(impl->window.get());
}

std::uint32_t Window::get_width() const { return impl->glfw_data.width; }
std::uint32_t Window::get_height() const { return impl->glfw_data.height; }

void Window::set_vsync(bool status) {
    glfwSwapInterval(status);
    impl->glfw_data.vsync = status;
}
bool Window::is_vsync() const { return true; }

}  // namespace k2