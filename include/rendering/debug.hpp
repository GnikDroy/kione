#pragma once

#include "core/logger.hpp"

#include <format>
#include <glad/glad.h>
#include <string>

namespace k2 {

inline bool enable_debug() {
    // Not possible to enable debug if GLFW hasn't been set up that way.
    // Only available in debug modes.
#ifdef NDEBUG
    return false;
#else
    auto gl_debug = [](auto source, auto type, auto id, auto severity, auto length, auto message, auto) {
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
            return;

        auto source_str = [&]() {
            switch (source) {
            case GL_DEBUG_SOURCE_API: return "API";
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "Window";
            case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
            case GL_DEBUG_SOURCE_THIRD_PARTY: return "Third Party";
            case GL_DEBUG_SOURCE_APPLICATION: return "Application";
            default: return "Other";
            }
        }();

        auto type_str = [&]() {
            switch (type) {
            case GL_DEBUG_TYPE_ERROR: return "Error";
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated";
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "Undefined Behaviour";
            case GL_DEBUG_TYPE_PORTABILITY: return "Portability";
            case GL_DEBUG_TYPE_PERFORMANCE: return "Performance";
            case GL_DEBUG_TYPE_MARKER: return "Marker";
            case GL_DEBUG_TYPE_PUSH_GROUP: return "Push Group";
            case GL_DEBUG_TYPE_POP_GROUP: return "Pop Group";
            default: return "Other";
            }
        }();

        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            k2::Log::core().error(
                std::format("GL ({}) [{}] [{}]: {}", id, type_str, source_str, std::string(message, message + length)));
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            k2::Log::core().warn(
                std::format("GL ({}) [{}] [{}]: {}", id, type_str, source_str, std::string(message, message + length)));
            break;
        case GL_DEBUG_SEVERITY_LOW:
            k2::Log::core().info(
                std::format("GL ({}) [{}] [{}]: {}", id, type_str, source_str, std::string(message, message + length)));
            break;
        default:
            k2::Log::core().trace(
                std::format("GL ({}) [{}] [{}]: {}", id, type_str, source_str, std::string(message, message + length)));
        }
    };

    auto debug_set = []() {
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        return flags & GL_CONTEXT_FLAG_DEBUG_BIT;
    }();

    // glDebugMessageCallback requires GL 4.3 :(
    if (debug_set && glDebugMessageCallback != nullptr) {
        k2::Log::core().info("GL Debug handler injected.");
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(gl_debug, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    } else {
        k2::Log::core().info("GL Debug handler NOT present.");
    }
    return debug_set;
#endif
}
}
