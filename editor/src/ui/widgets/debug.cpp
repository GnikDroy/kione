#include "ui/widgets/debug.hpp"

#include "components/camera.hpp"
#include "components/light.hpp"
#include "components/script.hpp"
#include "components/sprite.hpp"
#include "editor_layer.hpp"

namespace k2::editor {

void DebugWidget::render(EditorLayer& editor_layer) {
    auto& io = ImGui::GetIO();
    auto& registry = editor_layer.active_scene().registry;

    ImGui::Text("%.1f FPS  (%.2f ms/frame)", io.Framerate, 1000.0f / io.Framerate);
    ImGui::Text("Mode: %s", editor_layer.is_playing() ? "Playing" : "Editing");
    ImGui::Separator();

    std::size_t entities = 0;
    for ([[maybe_unused]] auto entity : registry.view<entt::entity>()) {
        entities++;
    }

    if (ImGui::BeginTable("##stats", 2)) {
        auto row = [](const char* label, std::size_t count) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(label);
            ImGui::TableNextColumn();
            ImGui::Text("%zu", count);
        };
        row("Entities", entities);
        row("Sprites", registry.view<SpriteComponent>().size());
        row("Scripts", registry.view<ScriptComponent>().size());
        row("Cameras", registry.view<Camera>().size());
        row("Point lights", registry.view<PointLight>().size());
        row("Spot lights", registry.view<SpotLight>().size());
        row("Sprite lights", registry.view<SpriteLight>().size());
        ImGui::EndTable();
    }
}
}
