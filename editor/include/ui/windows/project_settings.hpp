#pragma once

#include "ui/widgets/project_settings.hpp"
#include "ui/windows/window.hpp"

namespace k2::editor {
class ProjectSettingsWindow : public IImGuiWindow {
    ProjectSettingsWidget widget;

protected:
    void render_internal(EditorLayer& editor_layer) override { widget.render(editor_layer); }

public:
    explicit ProjectSettingsWindow(const std::string& title)
        : IImGuiWindow(title) { }
};

}
