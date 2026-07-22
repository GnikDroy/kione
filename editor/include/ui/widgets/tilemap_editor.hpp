#pragma once
#include "components/tilemap.hpp"
#include "core/scene.hpp"
#include "rendering/renderer2D.hpp"
#include "ui/widgets/widget.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

namespace k2::editor {
class TileMapEditorWidget : public IWidget {
    k2::Renderer2D renderer2D;
    k2::Scene canvas;
    entt::entity canvas_entity { entt::null };

    float width = 100.0f;
    float height = 100.0f;
    glm::vec2 camera_position {};
    float zoom = 1.0f;
    entt::entity framed_for { entt::null };

    enum class Tool : std::uint8_t { Brush, Eraser, Fill };
    Tool tool = Tool::Brush;
    int active_tile = 0;
    float palette_zoom = 1.0f;
    glm::ivec2 pending_size { 16, 16 };

    void resize_frame_buffer();
    void update_camera();
    [[nodiscard]] k2::Rect<float> viewport(ImVec2 rect_min) const {
        return { .x = rect_min.x, .y = rect_min.y, .w = width, .h = height };
    }
    void draw_canvas(EditorLayer&, entt::entity active, k2::TileMapComponent& tilemap);
    void handle_paint(k2::TileMapComponent& tilemap, ImVec2 rect_min);
    void push_grid_overlay(const k2::TileMapComponent& tilemap, ImVec2 rect_min);
    void draw_side_panel(EditorLayer&, k2::TileMapComponent& tilemap);

public:
    TileMapEditorWidget();
    void render(EditorLayer&) override;
};
}
