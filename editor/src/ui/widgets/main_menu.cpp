
#include "ui/widgets/main_menu.hpp"
#include "components/relation.hpp"
#include "editor_layer.hpp"

#include "serializers/core/scene.hpp" // IWYU pragma: keep

#include <IconsFontAwesome5.h>
#include <algorithm>
#include <nfd.hpp>

namespace k2::editor {

static void new_project_dialog(EditorLayer& editor_layer) {
    std::array filters = { nfdfilteritem_t { "Kione project", "k2project" } };
    [[maybe_unused]] auto lock = NFD::Guard();
    NFD::UniquePathU8 path;
    if (NFD::SaveDialog(path, filters.data(), nfdfiltersize_t(filters.size())) != NFD_OKAY) {
        return;
    }
    if (auto created = editor_layer.create_project(std::filesystem::path { path.get() })) {
        Log::core().info(std::format("Created project: {}", std::string_view { path.get() }));
    } else {
        Log::core().error(std::format("Failed to create project: {}", created.error()));
    }
}

static void open_project_dialog(EditorLayer& editor_layer) {
    std::array filters = { nfdfilteritem_t { "Kione project", "k2project" } };
    [[maybe_unused]] auto lock = NFD::Guard();
    NFD::UniquePathU8 path;
    if (NFD::OpenDialog(path, filters.data(), nfdfiltersize_t(filters.size())) != NFD_OKAY) {
        return;
    }
    if (auto opened = editor_layer.open_project(std::filesystem::path { path.get() })) {
        Log::core().info(std::format("Opened project: {}", std::string_view { path.get() }));
    } else {
        Log::core().error(std::format("Failed to open project: {}", opened.error()));
    }
}

static void save_scene(EditorLayer& editor_layer) {
    if (!editor_layer.project.has_value()) {
        return;
    }
    try {
        auto it = editor_layer.project->assets.find(ResourceManager::resolve(editor_layer.current_scene));
        if (it == editor_layer.project->assets.end() || it->second.second.type != Asset::Type::Scene) {
            Log::core().error(
                std::format("Project has no scene asset '{}' to save to", editor_layer.current_scene));
            return;
        }
        auto path = std::string { it->second.second.get_url_divisions().path };
        std::ofstream scene_file_stream { path };
        scene_file_stream << YAML::Node(editor_layer.scene);
        if (scene_file_stream) {
            Log::core().info(std::format("Saved scene: {}", path));
        } else {
            Log::core().error(std::format("Failed to save scene: {}", path));
        }
    } catch (const std::exception& e) {
        Log::core().error(std::format("Failed to save scene: {}", e.what()));
    }
}

static void save_scene_as(EditorLayer& editor_layer) {
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

static void new_scene_dialog(EditorLayer& editor_layer) {
    std::array filters = { nfdfilteritem_t { "Scene files", "k2scene" } };
    [[maybe_unused]] auto lock = NFD::Guard();
    NFD::UniquePathU8 path;
    auto default_path = editor_layer.project->root.string();
    if (NFD::SaveDialog(path, filters.data(), nfdfiltersize_t(filters.size()), default_path.c_str()) != NFD_OKAY) {
        return;
    }
    if (auto created = editor_layer.create_scene(std::filesystem::path { path.get() })) {
        Log::core().info(std::format("Created scene: {}", std::string_view { path.get() }));
    } else {
        Log::core().error(std::format("Failed to create scene: {}", created.error()));
    }
}

static void toggle_play(EditorLayer& editor_layer) {
    editor_layer.is_playing() ? editor_layer.stop() : editor_layer.play();
}

static void render_file_menu(EditorLayer& editor_layer) {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Project", "CTRL+N")) {
            new_project_dialog(editor_layer);
        }
        if (ImGui::MenuItem("Open Project", "CTRL+O")) {
            open_project_dialog(editor_layer);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Save", "CTRL+S", false, editor_layer.project.has_value())) {
            save_scene(editor_layer);
        }
        if (ImGui::MenuItem("Save As", "CTRL+SHIFT+S")) {
            save_scene_as(editor_layer);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) {
            editor_layer.request_exit();
        }
        ImGui::EndMenu();
    }
}

static void render_scene_menu(EditorLayer& editor_layer) {
    if (ImGui::BeginMenu("Scene")) {
        bool playing = editor_layer.is_playing();
        if (ImGui::MenuItem(ICON_FA_PLAY "  Play", "CTRL+P", false, !playing)) {
            editor_layer.play();
        }
        if (ImGui::MenuItem(ICON_FA_STOP "  Stop", "CTRL+P", false, playing)) {
            editor_layer.stop();
        }
        if (ImGui::MenuItem(ICON_FA_PLUS "  New Scene", nullptr, false,
                editor_layer.project.has_value() && !playing)) {
            new_scene_dialog(editor_layer);
        }
        if (ImGui::BeginMenu(ICON_FA_MAP "  Open Scene", editor_layer.project.has_value() && !playing)) {
            std::vector<std::string> scene_names;
            for (const auto& [id, pair] : editor_layer.active_assets()) {
                if (pair.second.type == Asset::Type::Scene) {
                    scene_names.push_back(pair.first);
                }
            }
            std::ranges::sort(scene_names);
            for (const auto& name : scene_names) {
                if (ImGui::MenuItem(name.c_str(), nullptr, name == editor_layer.current_scene)) {
                    if (auto opened = editor_layer.open_scene(name); !opened) {
                        Log::core().error(std::format("Failed to open scene '{}': {}", name, opened.error()));
                    }
                }
            }
            ImGui::EndMenu();
        }
        ImGui::Separator();

        auto& registry = editor_layer.active_scene().registry;
        auto& selector = editor_layer.entity_selector.get_widget();
        auto active = selector.get_active();
        bool has_selection = registry.valid(active);

        if (ImGui::MenuItem("Create Entity")) {
            selector.set_active(selector.create_entity(registry));
        }
        if (ImGui::MenuItem("Create Child", nullptr, false, has_selection)) {
            auto child = selector.create_entity(registry);
            RelationComponent::attach_last(registry, child, active);
            selector.set_active(child);
        }
        if (ImGui::MenuItem("Duplicate Entity", nullptr, false, has_selection)) {
            selector.set_active(duplicate_entity(editor_layer, registry, active));
        }
        if (ImGui::MenuItem("Delete Entity", nullptr, false, has_selection)) {
            RelationComponent::detach(registry, active);
            auto&& children = RelationComponent::get_children(registry, active, true);
            std::vector<entt::entity> to_delete;
            to_delete.reserve(children.size() + 1);
            std::ranges::transform(children, std::back_inserter(to_delete), [](auto& pair) { return pair.first; });
            to_delete.push_back(active);
            registry.destroy(to_delete.begin(), to_delete.end());
            selector.reset_selection();
        }
        ImGui::Separator();
        if (ImGui::BeginMenu("Add Component", has_selection)) {
            editor_layer.component_inspector.get_widget().add_component_menu_items(registry, active);
            ImGui::EndMenu();
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
        if (ImGui::MenuItem(editor_layer.project_settings.title.c_str(), "", editor_layer.project_settings.show)) {
            editor_layer.project_settings.show = !editor_layer.project_settings.show;
        }
        if (ImGui::MenuItem(editor_layer.asset_list.title.c_str(), "", editor_layer.asset_list.show)) {
            editor_layer.asset_list.show = !editor_layer.asset_list.show;
        }
        if (ImGui::MenuItem(editor_layer.animation_editor.title.c_str(), "", editor_layer.animation_editor.show)) {
            editor_layer.animation_editor.show = !editor_layer.animation_editor.show;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Reset Layout")) {
            editor_layer.reset_layout();
        }
        ImGui::EndMenu();
    }
}

void MainMenuWidget::render(EditorLayer& editor_layer) {
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_N)) {
        new_project_dialog(editor_layer);
    }
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_O)) {
        open_project_dialog(editor_layer);
    }
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)) {
        save_scene(editor_layer);
    }
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S)) {
        save_scene_as(editor_layer);
    }
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_P)) {
        toggle_play(editor_layer);
    }

    if (ImGui::BeginMainMenuBar()) {
        render_file_menu(editor_layer);
        render_scene_menu(editor_layer);
        render_view_menu(editor_layer);
        ImGui::EndMainMenuBar();
    }
}
}
