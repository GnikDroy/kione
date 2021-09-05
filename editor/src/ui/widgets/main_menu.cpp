#pragma once

#include "ui/widgets/main_menu.hpp"
#include "editor_layer.hpp"

#include "serializers/core/scene.hpp"
#include <cereal/archives/json.hpp>

#include <nfd.hpp>

namespace k2::editor {
static void render_file_menu(EditorLayer& editor_layer) {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open", "CTRL+O")) {
            std::array filters = { nfdfilteritem_t { "Scene files", "k2scene" } };
            NFD::UniquePathU8 path;
            if (NFD::OpenDialog(path, filters.data(), nfdfiltersize_t(filters.size())) == NFD_OKAY) {
                std::ifstream scene_file_stream { path.get() };
                cereal::JSONInputArchive archive { scene_file_stream };
                Scene new_scene;
                archive(new_scene);
                new_scene.registry.set<EditorLayer&>(editor_layer);
                editor_layer.scene = std::move(new_scene);
                Log::core().info(fmt::format("Opening file: {}", std::string_view { path.get() }));
            }
        }
        if (ImGui::MenuItem("Save As", "CTRL+SHIFT+S")) {
            std::array filters = { nfdfilteritem_t { "Scene files", "k2scene" } };
            NFD::UniquePathU8 path;
            if (NFD::SaveDialog(path, filters.data(), nfdfiltersize_t(filters.size())) == NFD_OKAY) {
                std::ofstream scene_file_stream { path.get() };
                cereal::JSONOutputArchive archive(scene_file_stream);
                archive(editor_layer.scene);
                Log::core().info(fmt::format("Saving file as: {}", std::string_view { path.get() }));
            }
        }
        ImGui::EndMenu();
    }
}

static void render_view_menu(EditorLayer& editor_layer) {
    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem(editor_layer.log_viewer.title.c_str(), "", editor_layer.log_viewer.show)) {
            editor_layer.log_viewer.show = !editor_layer.log_viewer.show;
        }
        if (ImGui::MenuItem(
                editor_layer.component_inspector.title.c_str(), "", editor_layer.component_inspector.show)) {
            editor_layer.component_inspector.show = !editor_layer.component_inspector.show;
        }
        if (ImGui::MenuItem(editor_layer.entity_selector.title.c_str(), "", editor_layer.entity_selector.show)) {
            editor_layer.entity_selector.show = !editor_layer.entity_selector.show;
        }
        if (ImGui::MenuItem(editor_layer.debug_widget.title.c_str(), "", editor_layer.debug_widget.show)) {
            editor_layer.debug_widget.show = !editor_layer.debug_widget.show;
        }
        if (ImGui::MenuItem(editor_layer.file_explorer.title.c_str(), "", editor_layer.file_explorer.show)) {
            editor_layer.file_explorer.show = !editor_layer.file_explorer.show;
        }
        if (ImGui::MenuItem(editor_layer.viewport2D.title.c_str(), "", editor_layer.viewport2D.show)) {
            editor_layer.viewport2D.show = !editor_layer.viewport2D.show;
        }
        ImGui::EndMenu();
    }
}

void MainMenuWidget::render(EditorLayer& editor_layer) {
    [[maybe_unused]] auto lock = NFD::Guard();
    if (ImGui::BeginMainMenuBar()) {
        render_file_menu(editor_layer);
        render_view_menu(editor_layer);
        ImGui::EndMainMenuBar();
    }
}
}