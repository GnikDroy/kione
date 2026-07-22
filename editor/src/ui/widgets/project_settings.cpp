#include "ui/widgets/project_settings.hpp"
#include "editor_layer.hpp"
#include "ui/common.hpp"

namespace k2::editor {

void ProjectSettingsWidget::render(EditorLayer& editor_layer) {
    if (!editor_layer.project.has_value()) {
        ImGui::TextDisabled("No project open.");
        return;
    }
    auto& project = *editor_layer.project;

    if (loaded_file != project.file) {
        loaded_file = project.file;
        name = project.name;
        main_scene.set(project.main_scene);
    }

    if (BeginPropertyTable("ProjectSettings")) {
        PropertyLabel("Project File");
        ImGui::TextDisabled("%s", project.file.filename().string().c_str());
        PropertyLabel("Root");
        ImGui::TextDisabled("%s", project.root.string().c_str());
        PropertyLabel("Name");
        ImGui::InputText("##Name", &name);
        PropertyLabel("Main Scene");
        ResourceInputWidget("##MainScene", main_scene, editor_layer.active_assets(), Asset::Type::Scene, false);
        EndPropertyTable();
    }

    ImGui::Spacing();
    if (ImGui::Button(ICON_MS_SAVE "  Save")) {
        project.name = name;
        project.main_scene = main_scene.name;
        if (auto saved = project.save()) {
            Log::core().info(std::format("Saved project: {}", project.file.string()));
        } else {
            Log::core().error(std::format("Failed to save project: {}", saved.error()));
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_SYNC "  Reload Project")) {
        if (auto opened = editor_layer.open_project(project.file); !opened) {
            Log::core().error(std::format("Failed to reload project: {}", opened.error()));
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Reloads the scene and assets from disk; unsaved scene changes are lost.");
    }
}
}
