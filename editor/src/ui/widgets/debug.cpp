#pragma once

#include "ui/widgets/debug.hpp"
#include "editor_layer.hpp"
#include <nfd.hpp>

namespace k2::editor {
void DebugWidget::render(EditorLayer&) {
    ImGui::Checkbox("Show Imgui Demo Window", &show_demo_window);
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }
    if (ImGui::Button("Save Dialogue")) {
        [[maybe_unused]] auto lock = NFD::Guard();
        std::array filters = { nfdfilteritem_t { "Scene files", "k2scene" } };
        NFD::UniquePathU8 path;
        if (NFD::SaveDialog(path, filters.data(), nfdfiltersize_t(filters.size())) == NFD_OKAY) {
            // do something here.
        }
    }
}
}