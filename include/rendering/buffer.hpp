#pragma once

#include <cassert>
#include <glad/glad.h>

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
        glNamedBufferStorage(handle, static_cast<GLsizeiptr>(size), nullptr, GL_DYNAMIC_STORAGE_BIT);
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
            glNamedBufferSubData(handle, offset, size, data);
        }
    }

    operator bool() const { return handle != 0; }

    auto get() const { return handle; }

    ~BasicBuffer() {
        if (handle)
            glDeleteBuffers(1, &handle);
    }
};

using VertexBuffer = BasicBuffer<GL_ARRAY_BUFFER>;
using IndexBuffer = BasicBuffer<GL_ELEMENT_ARRAY_BUFFER>;
using UniformBuffer = BasicBuffer<GL_UNIFORM_BUFFER>;
}
