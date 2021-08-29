#pragma once

#include "ui/widgets/debug.hpp"
#include "ui/windows/window.hpp"

namespace k2::editor {
class DebugWindow : public IImGuiWindow {
    DebugWidget widget;

protected:
    void render_internal(EditorLayer& editor_layer) override { widget.render(editor_layer); }

public:
    explicit DebugWindow(const std::string& title)
        : IImGuiWindow(title) { }
};

}