
#include <IconsFontAwesome5.h>
#include <cassert>
#include <imgui.h>

#include "components/relation.hpp"
#include "components/tag.hpp"
#include "editor_layer.hpp"
#include "ui/widgets/entity_selector.hpp"

namespace k2::editor {

template <bool AttachBefore, class EntityType>
static void in_between_drag_drop_target(entt::basic_registry<EntityType>& registry, EntityType entity) {
    // Before
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {});
    if constexpr (AttachBefore) {
        ImGui::InvisibleButton("ENTITY_REORDER_BTN_BEFORE", { -1.f, 3.f });
    } else {
        ImGui::InvisibleButton("ENTITY_REORDER_BTN_AFTER", { -1.f, 3.f });
    }
    ImGui::PopStyleVar();
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
            auto dragged_entity = static_cast<EntityType*>(payload->Data);
            if (entity != *dragged_entity) {
                RelationComponent::detach(registry, *dragged_entity);
                if constexpr (AttachBefore) {
                    RelationComponent::attach_before(registry, *dragged_entity, entity);
                } else {
                    RelationComponent::attach_after(registry, *dragged_entity, entity);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}

template <class EntityType>
static void entity_drag_drop_target(entt::basic_registry<EntityType>& registry, EntityType entity, TagComponent* tag) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 7.0f, 7.0f });
    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("ENTITY", &entity, sizeof(entity));
        if (tag && tag->tag[0] != '\0') {
            ImGui::TextUnformatted(tag->tag.data());
        } else {
            ImGui::Text("Entity %d", entt::to_integral(entity));
        }
        ImGui::EndDragDropSource();
    }
    ImGui::PopStyleVar();

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
            [[maybe_unused]] entt::entity* dragged_entity = static_cast<entt::entity*>(payload->Data);
            RelationComponent::detach(registry, *dragged_entity);
            RelationComponent::attach_last(registry, *dragged_entity, entity);
        }
        ImGui::EndDragDropTarget();
    }
}

template <class EntityType>
static void entity_context_menu(
    entt::basic_registry<EntityType>& registry, EntityType entity, std::vector<EntityType>& to_delete) {
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Create Child")) {
            auto child = registry.create();
            RelationComponent::attach_last(registry, child, entity);
        }
        if (ImGui::MenuItem("Detach")) {
            RelationComponent::detach(registry, entity);
        }
        if (ImGui::MenuItem("Delete")) {
            RelationComponent::detach(registry, entity);
            auto&& children = RelationComponent::get_children(registry, entity, true);
            to_delete.reserve(children.size() + 1);
            std::ranges::transform(children, std::back_inserter(to_delete), [](auto& pair) { return pair.first; });
            to_delete.push_back(entity);
        }
        ImGui::EndPopup();
    }
}

template <class EntityType>
static void recursive_draw(EntityType& entity_clicked, const EntityType& active_entity, EntityType entity,
    entt::basic_registry<EntityType>& registry, bool first_node, std::vector<EntityType>& to_delete) {
    ImGui::PushID((void*)(std::uintptr_t)entt::to_integral(entity));

    auto* relation = registry.template try_get<RelationComponent>(entity);
    ImGuiTreeNodeFlags flags
        = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (entity == active_entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (!relation || !relation->children) {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (first_node) {
        in_between_drag_drop_target<true>(registry, entity);
    }

    bool node_open {};
    auto* tag = registry.template try_get<TagComponent>(entity);
    if (tag && tag->tag[0] != '\0') {
        node_open = ImGui::TreeNodeEx(reinterpret_cast<const void*>((uintptr_t)entt::to_integral(entity)), flags,
            ICON_FA_CUBE "  %s", tag->tag.data());
    } else {
        node_open = ImGui::TreeNodeEx(reinterpret_cast<const void*>((uintptr_t)entt::to_integral(entity)), flags,
            ICON_FA_CUBE "  Entity %d", entt::to_integral(entity));
    }

    if (ImGui::IsItemClicked()) {
        entity_clicked = entity;
    }

    entity_drag_drop_target(registry, entity, tag);

    entity_context_menu(registry, entity, to_delete);

    if (node_open) {
        // Recurse here.
        if (relation && relation->children > 0) {
            auto curr = relation->first;
            for (std::size_t i {}; i < relation->children; i++) {
                auto& curr_relation = registry.template get<RelationComponent>(curr);
                recursive_draw(entity_clicked, active_entity, curr, registry, curr == relation->first, to_delete);
                curr = curr_relation.next;
            }
        }
    }

    in_between_drag_drop_target<false>(registry, entity);

    if (node_open) {
        ImGui::TreePop();
    }
    ImGui::PopID();
}

template <class EntityType> void EntitySelector<EntityType>::render(EditorLayer& editor_layer) {
    auto& registry = editor_layer.scene.registry;
    if (!registry.valid(active_entity)) {
        active_entity = entt::null;
    }

    EntityType entity_clicked = active_entity;
    std::vector<EntityType> to_delete;
    for (auto entity : registry.view<entt::entity>()) {
        auto* relation = registry.try_get<RelationComponent>(entity);
        if (!relation || relation->parent == entt::null) {
            recursive_draw(entity_clicked, active_entity, entity, registry, false, to_delete);
        }
    }

    registry.destroy(to_delete.begin(), to_delete.end());
    active_entity = entity_clicked;

    if (ImGui::BeginPopupContextWindow(
            "##CreateEntityRoot", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
        // TODO: Create entity after last root entity
        if (ImGui::MenuItem("Create Entity")) {
            static_cast<void>(registry.create());
        }
        ImGui::EndPopup();
    }
}

// Instantiations
template void EntitySelector<entt::entity>::render(EditorLayer&);
}
