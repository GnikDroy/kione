#pragma once

#include "ui/widgets/log_viewer.hpp"
#include "ui/windows/window.hpp"

namespace k2::editor {

class LogViewerWindow : public IImGuiWindow {
    LogViewer widget;

protected:
    void render_internal(EditorLayer& editor_layer) override { widget.render(editor_layer); }

public:
    explicit LogViewerWindow(const std::string& title)
        : IImGuiWindow(title) { }
};

}