#pragma once
#include <glad/glad.h>

#include "core/logger.hpp"
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
    bool is_swap_chain_target() const { return id == 0; }
    const std::uint32_t get_id() const { return id; }
    const Traits& get_traits() const { return traits; }
    ~FrameBuffer() { glDeleteFramebuffers(1, &id); }

    FrameBuffer(Traits traits)
        : traits(std::move(traits)) {
        // Do nothing for a SwapChainTarget
        if (this->traits.attachments.empty()) {
            return;
        }
        generate_attachments_if_empty();
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        for (auto& attachment : this->traits.attachments) {
            auto type = [&]() {
                if (attachment.type == Attachment::Type::Color) {
                    return GL_COLOR_ATTACHMENT0;
                } else if (attachment.type == Attachment::Type::DepthStencil) {
                    return GL_DEPTH_STENCIL_ATTACHMENT;
                } else if (attachment.type == Attachment::Type::Depth) {
                    return GL_DEPTH_ATTACHMENT;
                } else if (attachment.type == Attachment::Type::Stencil) {
                    return GL_STENCIL_ATTACHMENT;
                } else {
                    Log::core().warn("Invalid attachment type. Assuming color attachment.");
                }
                return GL_COLOR_ATTACHMENT0;
            }();
            if (attachment.buffer_type == Attachment::BufferType::Texture) {
                glFramebufferTexture2D(GL_FRAMEBUFFER, type + attachment.index, GL_TEXTURE_2D, attachment.id, 0);
            } else if (attachment.buffer_type == Attachment::BufferType::RenderBuffer) {
                glNamedFramebufferRenderbuffer(id, type + attachment.index, GL_RENDERBUFFER, attachment.id);
            } else {
                Log::core().warn("Invalid buffer type for render buffer attachment. Skipping attachment.");
            }
        }
        auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            Log::core().error(std::format("Couldn't fully create framebuffer GL Ret: {}", status));
            throw std::runtime_error("Couldn't fully create framebuffer.");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

private:
    std::uint32_t id;
    Traits traits;

    std::vector<Texture2D> textures;
    std::vector<RenderBuffer> render_buffers;

    void generate_attachments_if_empty() {
        for (auto& attachment : traits.attachments) {
            if (attachment.id == 0) {
                auto format = [&]() {
                    switch (attachment.type) {
                    case Attachment::Type::Color: return GL_RGBA8;
                    case Attachment::Type::Depth: return GL_DEPTH_COMPONENT32;
                    case Attachment::Type::Stencil: return GL_STENCIL_INDEX8;
                    case Attachment::Type::DepthStencil: return GL_DEPTH24_STENCIL8;
                    default: {
                        Log::core().error("Cannot generate attachment of given type.");
                        throw std::invalid_argument("Cannot generate attachment of given type.");
                    }
                    }
                }();
                if (attachment.buffer_type == Attachment::BufferType::Texture) {
                    textures.emplace_back(traits.width, traits.height, std::span<std::uint8_t> {}, format, false);
                    attachment.id = textures.back().id;
                } else if (attachment.buffer_type == Attachment::BufferType::RenderBuffer) {
                    render_buffers.emplace_back(traits.width, traits.height, format);
                    attachment.id = render_buffers.back().get_id();
                }
            }
        }
    }
};
}
