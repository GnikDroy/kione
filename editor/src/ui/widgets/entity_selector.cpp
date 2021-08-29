#pragma once

#include "ui/widgets/entity_selector.hpp"
#include "editor_layer.hpp"

namespace k2::editor {

template <class EntityType> void EntitySelector<EntityType>::render(EditorLayer& editor_layer) {
    auto& registry = editor_layer.scene.registry;
    if (!registry.valid(active_entity)) {
        active_entity = entt::null;
    }

    EntityType entity_clicked = active_entity;
    registry.each([&](auto entity) {
        ImGuiTreeNodeFlags flags
            = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        if (entity == active_entity)
            flags |= ImGuiTreeNodeFlags_Selected;

        bool node_open {};
        auto* tag = registry.try_get<TagComponent>(entity);
        if (tag && !tag->str().empty()) {
            node_open = ImGui::TreeNodeEx(
                reinterpret_cast<const void*>((uintptr_t)entt::to_integral(entity)), flags, "%s", tag->tag.data());
        } else {
            node_open = ImGui::TreeNodeEx(reinterpret_cast<const void*>((uintptr_t)entt::to_integral(entity)), flags,
                "Entity %d", entt::to_integral(entity));
        }
        if (ImGui::IsItemClicked())
            entity_clicked = entity;

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete"))
                registry.destroy(entity);
            ImGui::EndPopup();
        }

        if (node_open) {
            ImGui::BulletText("This node is open");
            ImGui::TreePop();
        }
    });
    active_entity = entity_clicked;

    // Add new entity
    if (ImGui::BeginPopupContextWindow(
            "##CreateEntityPopup", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Create Entity")) {
            static_cast<void>(registry.create());
        }
        ImGui::EndPopup();
    }
}

// Instantiations
template void EntitySelector<entt::entity>::render(EditorLayer&);
}
