#include "ui/widgets/component_inspector.hpp"
#include "editor_layer.hpp"
#include "ui/components.hpp" // IWYU pragma: keep

namespace k2::editor {

template <class EntityType> void ComponentInspectorWidget<EntityType>::render(EditorLayer& layer) {
    auto& registry = layer.active_scene().registry;
    auto entity = layer.entity_selector.get_widget().get_active();
    if (!registry.valid(entity)) {
        return;
    }

    ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
    std::vector<const std::pair<ComponentTypeID, ComponentInfo>*> missing;
    for (auto& entry : component_infos) {
        auto& [component_type_id, ci] = entry;
        if (!entity_has_component(registry, entity, component_type_id)) {
            missing.push_back(&entry);
            continue;
        }

        ImGui::PushID(component_type_id);
        auto avail = ImGui::GetContentRegionAvail().x;
        bool open = ImGui::CollapsingHeader(
            ci.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);

        auto trash_width = ImGui::CalcTextSize(ICON_MS_DELETE).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        ImGui::SameLine(avail - trash_width);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {});
        bool removed = ImGui::Button(ICON_MS_DELETE);
        ImGui::PopStyleColor();
        if (removed) {
            ci.destroy(registry, entity);
            ImGui::PopID();
            continue;
        }

        if (open) {
            ImGui::Indent(12.0f);
            ImGui::PushID("Widget");
            ci.widget(registry, entity);
            ImGui::PopID();
            ImGui::Unindent(12.0f);
            ImGui::Spacing();
        }
        ImGui::PopID();
    }

    ImGui::Spacing();
    if (!missing.empty()) {
        if (ImGui::Button(ICON_MS_ADD_BOX "  Add Component", { -std::numeric_limits<float>::min(), 0.0f })) {
            ImGui::OpenPopup("##AddComponent");
        }

        if (ImGui::BeginPopup("##AddComponent")) {
            for (const auto* entry : missing) {
                ImGui::PushID(entry->first);
                if (ImGui::Selectable(entry->second.name.c_str())) {
                    entry->second.create(registry, entity);
                }
                ImGui::PopID();
            }
            ImGui::EndPopup();
        }
    }
    ImGui::PopID();
}

template <class EntityType> ComponentInspectorWidget<EntityType>::ComponentInspectorWidget() {
    register_component<k2::TagComponent>(ICON_MS_LABEL "  Tag");
    register_component<k2::TransformComponent>(ICON_MS_OPEN_WITH "  Transform");
    register_component<k2::SpriteComponent>(ICON_MS_IMAGE "  Sprite");
    register_component<k2::TextComponent>(ICON_MS_FONT_DOWNLOAD "  Text");
    register_component<k2::AnimationComponent>(ICON_MS_MOVIE "  Animation");
    register_component<k2::AudioSourceComponent>(ICON_MS_VOLUME_UP "  Audio Source");
    register_component<k2::ColliderComponent>(ICON_MS_ADJUST "  Collider");
    register_component<k2::Environment>(ICON_MS_CLOUD "  Environment");
    register_component<k2::Camera>(ICON_MS_PHOTO_CAMERA "  Camera");
    register_component<k2::MainCamera>(ICON_MS_VIDEOCAM "  Main Camera");
    register_component<k2::ScriptComponent>(ICON_MS_CODE "  Script");
    register_component<k2::TileMapComponent>(ICON_MS_GRID_ON "  Tile Map");
    register_component<k2::PointLight>(ICON_MS_LIGHTBULB "  Point Light");
    register_component<k2::SpotLight>(ICON_MS_STAR "  Spot Light");
    register_component<k2::SpriteLight>(ICON_MS_DARK_MODE "  Sprite Light");
}

// Instantiations
template ComponentInspectorWidget<entt::entity>::ComponentInspectorWidget();
template void ComponentInspectorWidget<entt::entity>::render(EditorLayer&);
}
