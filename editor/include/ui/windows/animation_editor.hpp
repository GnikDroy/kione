#pragma once

#include "ui/widgets/animation_editor.hpp"
#include "ui/windows/window.hpp"

namespace k2::editor {
class AnimationEditorWindow : public IImGuiWindow {
    AnimationEditorWidget widget;

protected:
    void render_internal(EditorLayer& editor_layer) override { widget.render(editor_layer); }

public:
    explicit AnimationEditorWindow(const std::string& title)
        : IImGuiWindow(title) { }
};

}
