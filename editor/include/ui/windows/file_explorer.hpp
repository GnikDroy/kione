#pragma once
#include "ui/widgets/file_explorer.hpp"
#include "ui/windows/window.hpp"

namespace k2::editor {
class FileExplorerWindow : public IImGuiWindow {
    FileExplorerWidget widget;

protected:
    void render_internal(EditorLayer& editor_layer) override { widget.render(editor_layer); }

public:
    explicit FileExplorerWindow(const std::string& title)
        : IImGuiWindow(title) { }
};

}