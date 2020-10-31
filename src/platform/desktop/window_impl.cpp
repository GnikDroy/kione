#include "glad/glad.h"
// glad before glfw
#include "core/logger.hpp"
#include "platform/desktop/window_impl.hpp"

namespace k2 {
Window::Impl::Impl(Window* win, const WindowConfig& config) {
    glfw_data = {
        .title = config.title,
        .width = config.width,
        .height = config.height,
        .vsync = true,
    };
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window.reset(glfwCreateWindow(glfw_data.width, glfw_data.height,
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
    glfwMakeContextCurrent(window.get());
    glfwSetWindowUserPointer(window.get(), win);
    glfwSwapInterval(glfw_data.vsync);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        k2::Logger::core->critical("Failed to initialize GLAD.");
    } else {
        k2::Logger::core->info("GLAD initialization successful.");
    }
}

Window::Impl::~Impl() { glfw_window_count--; }
}  // namespace k2