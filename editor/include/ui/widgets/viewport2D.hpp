#pragma once
#include "rendering/frame_buffer.hpp"
#include "rendering/renderer2D.hpp"
#include "ui/widgets/widget.hpp"
#include <imgui.h>

namespace k2::editor {
class Viewport2DWidget : public IWidget {
    float width, height;
    k2::Renderer2D renderer2D;

public:
    Viewport2DWidget();
    void render(EditorLayer&) override;
};
}