#pragma once
#include <cstdint>
#include <glad/glad.h>
#include <utility>
#include <vector>

#include "rendering/render_buffer.hpp"
#include "rendering/texture.hpp"

namespace k2 {
struct FrameBuffer {
    struct Attachment {
        enum class BufferType {
            Texture,
            RenderBuffer,
        };
        enum class Type {
            Color,
            Depth,
            Stencil,
            DepthStencil,
        };
        BufferType buffer_type = BufferType::Texture;
        Type type = Type::Color;
        std::uint32_t id {};
        std::uint32_t index {};
    };

    struct Traits {
        std::size_t width {}, height {};
        std::vector<Attachment> attachments;
    };

    FrameBuffer() = default;
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;

    FrameBuffer(FrameBuffer&& other) noexcept {
        std::swap(other.id, id);
        std::swap(other.traits, traits);
        std::swap(other.textures, textures);
        std::swap(other.render_buffers, render_buffers);
    }
    FrameBuffer& operator=(FrameBuffer&& other) noexcept {
        std::swap(other.id, id);
        std::swap(other.traits, traits);
        std::swap(other.textures, textures);
        std::swap(other.render_buffers, render_buffers);
        return *this;
    }
    [[nodiscard]] bool is_swap_chain_target() const { return id == 0; }
    [[nodiscard]] std::uint32_t get_id() const { return id; }
    [[nodiscard]] const Traits& get_traits() const { return traits; }
    ~FrameBuffer() { glDeleteFramebuffers(1, &id); }

    FrameBuffer(Traits traits);

private:
    std::uint32_t id {};
    Traits traits;

    std::vector<Texture2D> textures;
    std::vector<RenderBuffer> render_buffers;

    void generate_attachments_if_empty();
};
}
