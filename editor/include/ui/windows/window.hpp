#pragma once
#include <imgui.h>
#include <string>

namespace k2 {
class EditorLayer;
}

namespace k2::editor {
class IImGuiWindow {
protected:
    virtual void render_internal(EditorLayer& editor) = 0;

public:
    bool show = true;
    std::string title;

    explicit IImGuiWindow(std::string title, bool show = true)
        : show(show)
        , title(std::move(title)) { }

    virtual void render(k2::EditorLayer& editor_layer) {
        if (show) {
            if (ImGui::Begin(title.c_str(), &show)) {
                render_internal(editor_layer);
            }
            ImGui::End();
        }
    }

    virtual ~IImGuiWindow() = default;
};
}
