#pragma once
#include <cstdlib>
#include <format>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "core/logger.hpp"
#include "core/window.hpp"

namespace k2 {
struct Window::Impl {
    static inline std::uint32_t glfw_window_count = 0;

    static inline bool glfw_initialized = []() {
        glfwSetErrorCallback([](int error_code, const char* msg) {
            try {
                Log::core().error(std::format("GLFW Error {} : {}", error_code, msg));
            } catch (...) { }
        });
        std::atexit(glfwTerminate);
        // Runs before glfwTerminate at exit (reverse registration order), so the
        // callback never reaches the already-destroyed loggers during termination.
        std::atexit(+[] { glfwSetErrorCallback(nullptr); });
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
