#pragma once

#include "ui/widgets/tileset_editor.hpp"
#include "ui/windows/window.hpp"

namespace k2::editor {
class TileSetEditorWindow : public IImGuiWindow {
    TileSetEditorWidget widget;

protected:
    void render_internal(EditorLayer& editor_layer) override { widget.render(editor_layer); }

public:
    explicit TileSetEditorWindow(const std::string& title)
        : IImGuiWindow(title) { }
};

}
