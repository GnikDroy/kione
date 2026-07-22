
#include <IconsFontAwesome5.h>
#include <cassert>
#include <functional>
#include <imgui.h>

#include "components/relation.hpp"
#include "core/entity_ops.hpp"
#include "components/tag.hpp"
#include "components/transform.hpp"
#include "editor_layer.hpp"
#include "ui/widgets/entity_selector.hpp"

namespace k2::editor {

using DeferredOp = std::function<void()>;

// Reparenting changes what local transform is relative to.
// Rewrite it so we keep world position.
template <class EntityType, class Reparent>
static void reparent_preserving_world(
    entt::basic_registry<EntityType>& registry, EntityType entity, Reparent&& reparent) {
    auto* transform = registry.template try_get<TransformComponent>(entity);
    auto world = transform ? TransformComponent::world(registry, entity) : glm::mat4 { 1.0f };
    reparent();
    if (transform != nullptr) {
        transform->set_from_matrix(glm::inverse(TransformComponent::parent_world(registry, entity)) * world);
    }
}

static entt::entity duplicate_subtree(EditorLayer& editor_layer, entt::registry& registry, entt::entity source) {
    auto clone = registry.create();
    editor_layer.component_inspector.get_widget().copy_components(registry, source, clone);
    for (auto& [child, relation] : RelationComponent::get_children(registry, source)) {
        RelationComponent::attach_last(registry, duplicate_subtree(editor_layer, registry, child), clone);
    }
    return clone;
}

entt::entity duplicate_entity(EditorLayer& editor_layer, entt::registry& registry, entt::entity source) {
    auto clone = duplicate_subtree(editor_layer, registry, source);
    RelationComponent::attach_after(registry, clone, source);
    return clone;
}

template <bool AttachBefore, class EntityType>
static void in_between_drag_drop_target(
    entt::basic_registry<EntityType>& registry, EntityType entity, DeferredOp& deferred_op) {
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
            auto dragged = *static_cast<EntityType*>(payload->Data);
            deferred_op = [&registry, dragged, entity]() {
                if (RelationComponent::is_ancestor_or_self(registry, dragged, entity)) {
                    return;
                }
                reparent_preserving_world(registry, dragged, [&] {
                    if constexpr (AttachBefore) {
                        RelationComponent::attach_before(registry, dragged, entity);
                    } else {
                        RelationComponent::attach_after(registry, dragged, entity);
                    }
                });
            };
        }
        ImGui::EndDragDropTarget();
    }
}

template <class EntityType>
static void entity_drag_drop_target(
    entt::basic_registry<EntityType>& registry, EntityType entity, TagComponent* tag, DeferredOp& deferred_op) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2 { 7.0f, 7.0f });
    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("ENTITY", &entity, sizeof(entity));
        if (tag && !tag->tag.empty()) {
            ImGui::TextUnformatted(tag->tag.c_str());
        } else {
            ImGui::Text("Entity %d", entt::to_integral(entity));
        }
        ImGui::EndDragDropSource();
    }
    ImGui::PopStyleVar();

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY")) {
            auto dragged = *static_cast<EntityType*>(payload->Data);
            deferred_op = [&registry, dragged, entity]() {
                if (RelationComponent::is_ancestor_or_self(registry, dragged, entity)) {
                    return;
                }
                reparent_preserving_world(registry, dragged, [&] {
                    RelationComponent::attach_last(registry, dragged, entity);
                });
            };
        }
        ImGui::EndDragDropTarget();
    }
}

template <class EntityType>
static void entity_context_menu(EditorLayer& editor_layer, entt::basic_registry<EntityType>& registry,
    EntityType entity, DeferredOp& deferred_op) {
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Create Child")) {
            deferred_op = [&registry, entity]() {
                auto child = create_entity(registry);
                RelationComponent::attach_last(registry, child, entity);
            };
        }
        if (ImGui::MenuItem("Duplicate")) {
            deferred_op = [&editor_layer, &registry, entity]() {
                auto clone = duplicate_entity(editor_layer, registry, entity);
                editor_layer.entity_selector.get_widget().set_active(clone);
            };
        }
        if (ImGui::MenuItem("Detach")) {
            deferred_op = [&registry, entity]() {
                reparent_preserving_world(registry, entity, [&] {
                    RelationComponent::attach_last(registry, entity, k2::scene_root(registry));
                });
            };
        }
        if (ImGui::MenuItem("Delete")) {
            deferred_op = [&registry, entity]() { k2::destroy_with_children(registry, entity); };
        }
        ImGui::EndPopup();
    }
}

template <class EntityType>
static void recursive_draw(EditorLayer& editor_layer, EntityType& entity_clicked, const EntityType& active_entity,
    EntityType entity, entt::basic_registry<EntityType>& registry, bool first_node, DeferredOp& deferred_op) {
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
        in_between_drag_drop_target<true>(registry, entity, deferred_op);
    }

    bool node_open {};
    auto* tag = registry.template try_get<TagComponent>(entity);
    if (tag && !tag->tag.empty()) {
        node_open = ImGui::TreeNodeEx(reinterpret_cast<const void*>((uintptr_t)entt::to_integral(entity)), flags,
            ICON_FA_CUBE "  %s", tag->tag.c_str());
    } else {
        node_open = ImGui::TreeNodeEx(reinterpret_cast<const void*>((uintptr_t)entt::to_integral(entity)), flags,
            ICON_FA_CUBE "  Entity %d", entt::to_integral(entity));
    }

    if (ImGui::IsItemClicked()) {
        entity_clicked = entity;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
        && registry.template all_of<TransformComponent>(entity)) {
        auto world = TransformComponent::world(registry, entity);
        editor_layer.viewport2D.get_widget().focus({ world[3][0], world[3][1] });
    }

    entity_drag_drop_target(registry, entity, tag, deferred_op);

    entity_context_menu(editor_layer, registry, entity, deferred_op);

    if (node_open) {
        // Recurse here.
        if (relation && relation->children > 0) {
            auto curr = relation->first;
            for (std::size_t i {}; i < relation->children; i++) {
                auto& curr_relation = registry.template get<RelationComponent>(curr);
                recursive_draw(editor_layer, entity_clicked, active_entity, curr, registry,
                    curr == relation->first, deferred_op);
                curr = curr_relation.next;
            }
        }
    }

    in_between_drag_drop_target<false>(registry, entity, deferred_op);

    if (node_open) {
        ImGui::TreePop();
    }
    ImGui::PopID();
}

template <class EntityType> void EntitySelector<EntityType>::render(EditorLayer& editor_layer) {
    auto& registry = editor_layer.active_scene().registry;
    if (!registry.valid(active_entity)) {
        active_entity = entt::null;
    }

    if (ImGui::Button(ICON_FA_PLUS "  Create Entity", { -std::numeric_limits<float>::min(), 0.0f })) {
        active_entity = create_entity(registry);
    }
    ImGui::Separator();

    EntityType entity_clicked = active_entity;
    DeferredOp deferred_op;
    auto roots = RelationComponent::get_children(registry, k2::scene_root(registry));
    for (auto& [entity, relation] : roots) {
        recursive_draw(
            editor_layer, entity_clicked, active_entity, entity, registry, entity == roots.front().first, deferred_op);
    }

    if (deferred_op) {
        deferred_op();
    }
    active_entity = entity_clicked;

    if (ImGui::BeginPopupContextWindow(
            "##CreateEntityRoot", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Create Entity")) {
            active_entity = create_entity(registry);
        }
        ImGui::EndPopup();
    }
}

// Instantiations
template void EntitySelector<entt::entity>::render(EditorLayer&);
}
