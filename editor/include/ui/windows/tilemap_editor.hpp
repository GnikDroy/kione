#pragma once

#include "ui/widgets/tilemap_editor.hpp"
#include "ui/windows/window.hpp"

namespace k2::editor {
class TileMapEditorWindow : public IImGuiWindow {
    TileMapEditorWidget widget;

protected:
    void render_internal(EditorLayer& editor_layer) override { widget.render(editor_layer); }

public:
    explicit TileMapEditorWindow(const std::string& title)
        : IImGuiWindow(title) { }
};

}
