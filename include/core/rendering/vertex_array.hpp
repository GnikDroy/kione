#pragma once
#include "glad/glad.h"

namespace k2 {

    class VertexArray {
        GLuint handle{};

    public:
        VertexArray() {
            glGenVertexArrays(1, &handle);
        }

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;

        VertexArray(VertexArray&& other) noexcept { *this = std::move(other); }
        VertexArray& operator=(VertexArray&& other) noexcept {
            std::swap(handle, other.handle);
            return *this;
        }

        void bind() const {
            glBindVertexArray(handle);
        }

        operator bool() const { return handle != 0; }

        auto get() const { return handle; }

        static void unbind() { glBindVertexArray(0); }

        ~VertexArray() {
            if (handle) glDeleteVertexArrays(1, &handle);
        }
    };
}