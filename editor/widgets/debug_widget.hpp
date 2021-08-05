#pragma once

#include <imgui.h>

namespace k2::editor {
class DebugWidget {
    bool show_demo_window = false;

public:
    void render() {
        ImGui::Checkbox("Show Imgui Demo Window", &show_demo_window);
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
    }
};

}