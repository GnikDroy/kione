#pragma once

#include "core/utils.hpp"
#include "shader.hpp"

#include <cassert>
#include <glad/glad.h>

namespace k2 {

struct VertexAttributeTraits {
    size_t location {};
    ShaderDataType data_type {};
    size_t offset {};
    bool normalized {};

    [[nodiscard]] constexpr size_t num() const {
        switch (data_type) {
        case ShaderDataType::Int: return 1;
        case ShaderDataType::Int2: return 2;
        case ShaderDataType::Int3: return 3;
        case ShaderDataType::Int4: return 4;

        case ShaderDataType::Uint: return 1;
        case ShaderDataType::Uint2: return 2;
        case ShaderDataType::Uint3: return 3;
        case ShaderDataType::Uint4: return 4;

        case ShaderDataType::Float: return 1;
        case ShaderDataType::Float2: return 2;
        case ShaderDataType::Float3: return 3;
        case ShaderDataType::Float4: return 4;

        case ShaderDataType::Double: return 1;
        case ShaderDataType::Double2: return 2;
        case ShaderDataType::Double3: return 3;
        case ShaderDataType::Double4: return 4;

        case ShaderDataType::Bool: return 1;
        default: assert(false && "ShaderDataType not implemented.");
        }
        return 0;
    }

    [[nodiscard]] constexpr size_t size() const {
        switch (data_type) {
        case ShaderDataType::Int: return 4;
        case ShaderDataType::Int2: return 4 * 2;
        case ShaderDataType::Int3: return 4 * 3;
        case ShaderDataType::Int4: return 4 * 4;

        case ShaderDataType::Uint: return 4;
        case ShaderDataType::Uint2: return 4 * 2;
        case ShaderDataType::Uint3: return 4 * 3;
        case ShaderDataType::Uint4: return 4 * 4;

        case ShaderDataType::Float: return 4;
        case ShaderDataType::Float2: return 4 * 2;
        case ShaderDataType::Float3: return 4 * 3;
        case ShaderDataType::Float4: return 4 * 4;

        case ShaderDataType::Double: return 8;
        case ShaderDataType::Double2: return 8 * 2;
        case ShaderDataType::Double3: return 8 * 3;
        case ShaderDataType::Double4: return 8 * 4;

        case ShaderDataType::Mat2x2: return 4 * 2 * 2;
        case ShaderDataType::Mat3x3: return 4 * 3 * 3;
        case ShaderDataType::Mat4x4: return 4 * 4 * 4;

        case ShaderDataType::Bool: return 1;
        }
        assert(false && "ShaderDataType not implemented.");
        return 0;
    }

    constexpr auto type() const {
        switch (data_type) {
        case ShaderDataType::Int:
        case ShaderDataType::Int2:
        case ShaderDataType::Int3:
        case ShaderDataType::Int4: return GL_INT;

        case ShaderDataType::Uint:
        case ShaderDataType::Uint2:
        case ShaderDataType::Uint3:
        case ShaderDataType::Uint4: return GL_UNSIGNED_INT;

        case ShaderDataType::Float:
        case ShaderDataType::Float2:
        case ShaderDataType::Float3:
        case ShaderDataType::Float4: return GL_FLOAT;

        case ShaderDataType::Double:
        case ShaderDataType::Double2:
        case ShaderDataType::Double3:
        case ShaderDataType::Double4: return GL_DOUBLE;

        case ShaderDataType::Mat2x2: return GL_FLOAT_MAT2;

        case ShaderDataType::Mat3x3: return GL_FLOAT_MAT3;

        case ShaderDataType::Mat4x4: return GL_FLOAT_MAT4;

        case ShaderDataType::Bool: return GL_BOOL;
        }
        assert(false && "ShaderDataType not implemented.");
        return 0;
    }
};

struct VertexAttributeBinding {
    std::uint32_t buffer {};
    VertexAttributeTraits attribute_trait {};
};

class VertexArray {
    GLuint handle {};

public:
    VertexArray() { glGenVertexArrays(1, &handle); }

    VertexArray(const std::initializer_list<VertexAttributeBinding>& attribute_bindings, size_t stride,
        std::uint32_t index_buffer = {}) {
        glGenVertexArrays(1, &handle);
        apply(attribute_bindings, stride, index_buffer);
    }

    VertexArray(const VertexArray&) = delete;

    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&& other) noexcept { *this = std::move(other); }

    VertexArray& operator=(VertexArray&& other) noexcept {
        std::swap(handle, other.handle);
        return *this;
    }

    void bind() const { glBindVertexArray(handle); }

    void apply(const std::initializer_list<VertexAttributeBinding>& attribute_bindings, size_t stride) const {
        bind();
        for (const auto& attribute : attribute_bindings) {
            glBindBuffer(GL_ARRAY_BUFFER, attribute.buffer);
            glEnableVertexAttribArray((GLuint)attribute.attribute_trait.location);
            glVertexAttribPointer(static_cast<GLuint>(attribute.attribute_trait.location),
                static_cast<GLint>(attribute.attribute_trait.num()),
                static_cast<GLenum>(attribute.attribute_trait.type()),
                static_cast<GLboolean>(attribute.attribute_trait.normalized), static_cast<GLsizei>(stride),
                reinterpret_cast<void*>(attribute.attribute_trait.offset));
        }
        unbind();
    }

    void apply(const std::initializer_list<VertexAttributeBinding>& attribute_bindings, size_t stride,
        std::uint32_t index_buffer) {
        apply(attribute_bindings, stride);
        bind();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        unbind();
    }

    operator bool() const { return handle != 0; }

    auto get() const { return handle; }

    static void unbind() { glBindVertexArray(0); }

    ~VertexArray() {
        if (handle != 0u) {
            glDeleteVertexArrays(1, &handle);
        }
    }
};
}