#pragma once
#include <cstdlib>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "core/window.hpp"

namespace k2 {
struct Window::Impl {
    static inline std::uint32_t glfw_window_count = 0;

    [[maybe_unused]] static inline bool glfw_initialized = []() {
        std::atexit(glfwTerminate);
        return glfwInit() == GLFW_TRUE;
    }();

    std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window { nullptr, &glfwDestroyWindow };

    struct GLFWData {
        std::string title;
        bool vsync;
    } glfw_data;

    Impl(Window* instance, const WindowConfig& config);

    ~Impl();
};
} // namespace k2
