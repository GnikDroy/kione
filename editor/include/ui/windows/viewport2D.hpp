#pragma once

#include "ui/widgets/viewport2D.hpp"
#include "ui/windows/window.hpp"

namespace k2::editor {

class Viewport2DWindow : public IImGuiWindow {
    Viewport2DWidget widget;

protected:
    void render_internal(EditorLayer& editor_layer) override { widget.render(editor_layer); }

public:
    explicit Viewport2DWindow(const std::string& title)
        : IImGuiWindow(title) { }
};

}