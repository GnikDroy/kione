#pragma once

#include "ui/widgets/main_menu.hpp"
#include <array>
#include <fmt/format.h>
#include <nfd.hpp>

namespace k2::editor {
void MainMenuWidget::render(EditorLayer&) {
    [[maybe_unused]] auto lock = NFD::Guard();
    if (ImGui::BeginMainMenuBar()) {
        render_file_menu();
        render_view_menu();
        ImGui::EndMainMenuBar();
    }
}

void MainMenuWidget::render_file_menu() {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open", "CTRL+O")) {
            std::array filters = { nfdfilteritem_t { "Scene files", "k2scene" } };
            NFD::UniquePathU8 path;
            if (NFD::OpenDialog(path, filters.data(), nfdfiltersize_t(filters.size())) == NFD_OKAY) {
                Log::core().info(fmt::format("Opening file: {}", std::string_view { path.get() }));
            }
        }
        if (ImGui::MenuItem("Save As", "CTRL+SHIFT+S")) {
            std::array filters = { nfdfilteritem_t { "Scene files", "k2scene" } };
            NFD::UniquePathU8 path;
            if (NFD::SaveDialog(path, filters.data(), nfdfiltersize_t(filters.size())) == NFD_OKAY) {
                Log::core().info(fmt::format("Saving file as: {}", std::string_view { path.get() }));
            }
        }
        ImGui::EndMenu();
    }
}

void MainMenuWidget::render_view_menu() {
    if (ImGui::BeginMenu("View")) {
        ImGui::EndMenu();
    }
}
}