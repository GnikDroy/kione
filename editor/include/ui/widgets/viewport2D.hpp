#pragma once
#include "rendering/renderer2D.hpp"
#include "ui/widgets/widget.hpp"
#include <glm/glm.hpp>
#include <imgui.h>

namespace k2::editor {
class Viewport2DWidget : public IWidget {
    float width, height;
    glm::vec2 camera_position {};
    float zoom = 1.0f;
    int gizmo_operation;
    k2::Renderer2D renderer2D;

public:
    Viewport2DWidget();
    void render(EditorLayer&) override;

    void focus(glm::vec2 world_position) { camera_position = world_position; }

private:
    bool draw_toolbar(EditorLayer&, ImVec2 rect_min);
    void resize_frame_buffer();
    void update_camera();
    void draw_gizmo(EditorLayer&, ImVec2 rect_min);
    void handle_interaction(EditorLayer&, ImVec2 rect_min);
    [[nodiscard]] glm::vec2 screen_to_world(ImVec2 screen, ImVec2 rect_min) const;
};
}
