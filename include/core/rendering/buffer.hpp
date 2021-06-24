#pragma once

#include "glad/glad.h"
#include <cassert>

namespace k2 {
template <auto Type> class BasicBuffer {
    GLuint handle {};
    size_t capacity {};

public:
    BasicBuffer() = default;

    explicit BasicBuffer(size_t size)
        : capacity { size } {
        assert(size != 0 && "Zero sized buffer.");
        glGenBuffers(1, &handle);
        glBindBuffer(Type, handle);
        glBufferData(Type, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_DRAW);
    }

    BasicBuffer(const BasicBuffer&) = delete;
    BasicBuffer& operator=(const BasicBuffer&) = delete;

    BasicBuffer(BasicBuffer&& other) noexcept { *this = std::move(other); }
    BasicBuffer& operator=(BasicBuffer&& other) noexcept {
        std::swap(handle, other.handle);
        capacity = other.capacity;
        return *this;
    }

    void set(const void* data, size_t size, size_t offset = 0) const {
        assert(size + offset <= capacity && "Not enough capacity.");
        if (capacity != 0) {
            glBindBuffer(Type, handle);
            glBufferSubData(Type, offset, size, data);
        }
    }

    operator bool() const { return handle != 0; }

    void bind() const { glBindBuffer(Type, handle); }

    auto get() const { return handle; }

    static void unbind() { glBindBuffer(Type, 0); }

    ~BasicBuffer() {
        if (handle)
            glDeleteBuffers(1, &handle);
    }
};

using VertexBuffer = BasicBuffer<GL_ARRAY_BUFFER>;
using IndexBuffer = BasicBuffer<GL_ELEMENT_ARRAY_BUFFER>;
}
