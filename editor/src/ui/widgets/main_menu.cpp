
#include "ui/widgets/main_menu.hpp"
#include "editor_layer.hpp"

#include "serializers/core/scene.hpp" // IWYU pragma: keep

#include <nfd.hpp>

namespace k2::editor {
static void render_file_menu(EditorLayer& editor_layer) {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open", "CTRL+O")) {
            try {
                std::array filters = { nfdfilteritem_t { "Scene files", "k2scene" } };
                [[maybe_unused]] auto lock = NFD::Guard();
                NFD::UniquePathU8 path;
                if (NFD::OpenDialog(path, filters.data(), nfdfiltersize_t(filters.size())) == NFD_OKAY) {
                    auto new_scene = YAML::LoadFile(path.get()).as<Scene>();
                    new_scene.registry.ctx().emplace<EditorLayer&>(editor_layer);
                    editor_layer.scene = std::move(new_scene);
                    Log::core().info(std::format("Opening file: {}", std::string_view { path.get() }));
                }
            } catch (const std::exception& e) {
                Log::core().error(std::format("Failed to open scene: {}", e.what()));
            }
        }
        if (ImGui::MenuItem("Save As", "CTRL+SHIFT+S")) {
            try {
                std::array filters = { nfdfilteritem_t { "Scene files", "k2scene" } };
                [[maybe_unused]] auto lock = NFD::Guard();
                NFD::UniquePathU8 path;
                if (NFD::SaveDialog(path, filters.data(), nfdfiltersize_t(filters.size())) == NFD_OKAY) {
                    std::ofstream scene_file_stream { path.get() };
                    scene_file_stream << YAML::Node(editor_layer.scene);
                    if (scene_file_stream) {
                        Log::core().info(std::format("Saving file as: {}", std::string_view { path.get() }));
                    } else {
                        Log::core().error(std::format("Failed to save file: {}", std::string_view { path.get() }));
                    }
                }
            } catch (const std::exception& e) {
                Log::core().error(std::format("Failed to save scene: {}", e.what()));
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
    if (ImGui::BeginMainMenuBar()) {
        render_file_menu(editor_layer);
        render_view_menu(editor_layer);
        ImGui::EndMainMenuBar();
    }
}
}
