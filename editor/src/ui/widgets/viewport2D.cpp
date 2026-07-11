#include "ui/widgets/viewport2D.hpp"
#include "editor_layer.hpp"
#include <algorithm>
#include <imgui.h>

namespace k2::editor {
Viewport2DWidget::Viewport2DWidget() {
    width = 100;
    height = 100;
    renderer2D.set_frame_buffer({ { .width = std::size_t(width),
        .height = std::size_t(height),
        .attachments {
            {
                .buffer_type = k2::FrameBuffer::Attachment::BufferType::Texture,
                .type = k2::FrameBuffer::Attachment::Type::Color,
            },
            {
                .buffer_type = k2::FrameBuffer::Attachment::BufferType::Texture,
                .type = k2::FrameBuffer::Attachment::Type::DepthStencil,
            },
        } } });

    renderer2D.camera = k2::Camera {
        .position { 0, 0, 1000.f },
        .target { 0, 0, 0 },
        .up { 0, 1.0f, 0 },

        .projection_traits { k2::Camera::OrthographicTraits {
            .left = -float(width),
            .right = float(width),
            .top = float(height),
            .bottom = -float(height),
            .far_clip = -1000.f,
            .near_clip = 1000.f,
        } },
    };
}

void Viewport2DWidget::render(EditorLayer& editor_layer) {
    auto& scene = editor_layer.scene;

    auto space = ImGui::GetContentRegionAvail();
    // Framebuffers cannot be 0x0
    space.x = std::max(1.f, space.x);
    space.y = std::max(1.f, space.y);

    bool changed = (space.x != width || space.y != height);
    width = space.x;
    height = space.y;

    if (changed) {
        auto new_traits = renderer2D.get_frame_buffer().get_traits();
        new_traits.width = (std::size_t)width;
        new_traits.height = (std::size_t)height;
        // Zero out invalidated id references, instructing to create new textures and render buffers.
        for (auto& attachment : new_traits.attachments) {
            attachment.id = 0;
        }
        renderer2D.set_frame_buffer({ new_traits });

        renderer2D.camera.projection_traits = k2::Camera::OrthographicTraits {
            .left = -width,
            .right = width,
            .top = height,
            .bottom = -height,
            .far_clip = -1000.f,
            .near_clip = 1000.f,
        };
    }

    renderer2D.set_clear_color(0.2f, 0.2f, 0.2f, 1.0f);
    renderer2D.clear();
    renderer2D.draw(scene);
    renderer2D.render();

    // Show the color attachment in viewport
    std::uint32_t texture_id = renderer2D.get_frame_buffer().get_traits().attachments.front().id;

    ImGui::Image((std::uint64_t)texture_id,
        { (float)renderer2D.get_frame_buffer().get_traits().width,
            (float)renderer2D.get_frame_buffer().get_traits().height },
        { 0, 1 }, { 1, 0 });
}
}
