#pragma once
#include <cstdint>
#include <glad/glad.h>
#include <utility>

namespace k2 {
class RenderBuffer {
    std::uint32_t id {};

public:
    RenderBuffer() = default;
    RenderBuffer(const RenderBuffer&) = delete;
    RenderBuffer& operator=(const RenderBuffer&) = delete;
    RenderBuffer(RenderBuffer&& other) noexcept { std::swap(id, other.id); }
    RenderBuffer& operator=(RenderBuffer&& other) noexcept {
        std::swap(id, other.id);
        return *this;
    }
    operator bool() const { return id != 0; }
    [[nodiscard]] std::uint32_t get_id() const { return id; }
    ~RenderBuffer() { glDeleteRenderbuffers(1, &id); }

    RenderBuffer(std::size_t width, std::size_t height, GLuint format = GL_RGBA8) {
        glGenRenderbuffers(1, &id);
        glBindRenderbuffer(GL_RENDERBUFFER, id);
        glRenderbufferStorage(GL_RENDERBUFFER, format, (GLsizei)width, (GLsizei)height);
    }
};

}
