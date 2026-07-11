#include "rendering/frame_buffer.hpp"

#include <format>
#include <span>
#include <stdexcept>

#include "core/logger.hpp"

namespace k2 {

FrameBuffer::FrameBuffer(Traits traits)
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
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, type + attachment.index, GL_RENDERBUFFER, attachment.id);
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

void FrameBuffer::generate_attachments_if_empty() {
    for (auto& attachment : traits.attachments) {
        if (attachment.id == 0) {
            auto format = [&]() {
                switch (attachment.type) {
                case Attachment::Type::Color: return GL_RGBA32F;
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
                textures.emplace_back(traits.width, traits.height, std::span<const float> {}, format, false);
                attachment.id = textures.back().id;
            } else if (attachment.buffer_type == Attachment::BufferType::RenderBuffer) {
                render_buffers.emplace_back(traits.width, traits.height, format);
                attachment.id = render_buffers.back().get_id();
            }
        }
    }
}

}
