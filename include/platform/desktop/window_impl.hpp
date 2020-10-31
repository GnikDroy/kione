#pragma once
#include <cstdlib>

#include "GLFW/glfw3.h"
#include "core/window.hpp"

namespace k2 {
struct Window::Impl {
    static inline std::uint32_t glfw_window_count = 0;
    
    static inline bool glfw_initialized = []() {
        bool initialized = glfwInit() == GLFW_TRUE;
        if (initialized) std::atexit([]() { glfwTerminate(); });
        return initialized;
    }();
    
    std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window{
        nullptr, &glfwDestroyWindow};

    struct GLFWData {
        std::string title;
        std::uint32_t width, height;
        bool vsync;
    } glfw_data;

    Impl(Window* instance, const WindowConfig& config);

    ~Impl();
};
}  // namespace k2