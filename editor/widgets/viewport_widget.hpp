#pragma once
#include "rendering/frame_buffer.hpp"
#include <imgui.h>

namespace k2::editor {

class ViewportWidget {
    float width, height;

public:
    void render(Renderer2D& renderer2D) {
        auto space = ImGui::GetContentRegionAvail();
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
        }

        std::uint32_t texture_id {};
        for (auto& attachment : renderer2D.get_frame_buffer().get_traits().attachments) {
            if (attachment.buffer_type == FrameBuffer::Attachment::BufferType::Texture) {
                texture_id = attachment.id;
                break;
            }
        }

        ImGui::Image((void*)(std::uintptr_t)texture_id,
            { (float)renderer2D.get_frame_buffer().get_traits().width,
                (float)renderer2D.get_frame_buffer().get_traits().height },
            { 0, 1 }, { 1, 0 });
    }
};

}