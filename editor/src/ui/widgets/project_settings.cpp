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
        main_scene = std::filesystem::relative(project.main_scene, project.root).generic_string();
    }

    if (BeginPropertyTable("ProjectSettings")) {
        PropertyLabel("Project File");
        ImGui::TextDisabled("%s", project.file.filename().string().c_str());
        PropertyLabel("Root");
        ImGui::TextDisabled("%s", project.root.string().c_str());
        PropertyLabel("Name");
        ImGui::InputText("##Name", &name);
        PropertyLabel("Main Scene");
        ImGui::InputText("##MainScene", &main_scene);
        EndPropertyTable();
    }

    ImGui::Spacing();
    if (ImGui::Button(ICON_FA_SAVE "  Save")) {
        project.name = name;
        project.main_scene = project.root / main_scene;
        if (auto saved = project.save()) {
            Log::core().info(std::format("Saved project: {}", project.file.string()));
        } else {
            Log::core().error(std::format("Failed to save project: {}", saved.error()));
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_SYNC "  Reload Project")) {
        try {
            editor_layer.open_project(project.file);
        } catch (const std::exception& e) {
            Log::core().error(std::format("Failed to reload project: {}", e.what()));
        }
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Reloads the scene and assets from disk; unsaved scene changes are lost.");
    }
}
}
