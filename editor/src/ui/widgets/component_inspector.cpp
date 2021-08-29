#pragma once

#include "ui/widgets/component_inspector.hpp"
#include "editor_layer.hpp"
#include "ui/components.hpp"

namespace k2::editor {

template <class EntityType> void ComponentInspectorWidget<EntityType>::render(EditorLayer& layer) {
    auto& registry = layer.scene.registry;
    auto entity = layer.entity_selector.get_widget().get_active();

    if (registry.valid(entity)) {
        ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
        std::map<ComponentTypeID, ComponentInfo> has_not;
        for (auto& [component_type_id, ci] : component_infos) {
            if (entity_has_component(registry, entity, component_type_id)) {
                ImGui::PushID(component_type_id);
                if (ImGui::Button(ICON_FA_TRASH)) {
                    ci.destroy(registry, entity);
                    ImGui::PopID();
                    continue; // early out to prevent access to deleted data
                } else {
                    ImGui::SameLine();
                }

                if (ImGui::CollapsingHeader(ci.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Indent(30.f);
                    ImGui::PushID("Widget");
                    ci.widget(registry, entity);
                    ImGui::PopID();
                    ImGui::Unindent(30.f);
                }
                ImGui::PopID();
            } else {
                has_not[component_type_id] = ci;
            }
        }

        ImGui::Separator();
        if (!has_not.empty()) {
            if (ImGui::Button(ICON_FA_PLUS_SQUARE "  Add Component")) {
                ImGui::OpenPopup(ICON_FA_PLUS_SQUARE "  Add Component");
            }

            if (ImGui::BeginPopup(ICON_FA_PLUS_SQUARE "  Add Component")) {
                ImGui::TextUnformatted("Available:");
                ImGui::Separator();

                for (auto& [component_type_id, ci] : has_not) {
                    ImGui::PushID(component_type_id);
                    if (ImGui::Selectable(ci.name.c_str())) {
                        ci.create(registry, entity);
                    }
                    ImGui::PopID();
                }
                ImGui::EndPopup();
            }
        }
        ImGui::PopID();
    }
}

template <class EntityType> ComponentInspectorWidget<EntityType>::ComponentInspectorWidget() {
    register_component<k2::TransformComponent>(ICON_FA_ARROWS_ALT "  Transform");
    register_component<k2::Camera>(ICON_FA_CAMERA "  Camera");
    register_component<k2::SpriteComponent>(ICON_FA_IMAGE "  Sprite");
    register_component<k2::TagComponent>(ICON_FA_TAGS "  Tag");
}

// Instantiations
template ComponentInspectorWidget<entt::entity>::ComponentInspectorWidget();
template void ComponentInspectorWidget<entt::entity>::render(EditorLayer&);
}
