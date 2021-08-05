#pragma once

#include <cassert>
#include <functional>
#include <map>
#include <set>
#include <string>

#include <IconsFontAwesome5.h>
#include <entt/entt.hpp>
#include <imgui.h>

namespace k2 {

template <class EntityType> class EntitySelector {
    EntityType active_entity { entt::null };

public:
    using Registry = entt::basic_registry<EntityType>;

    void render(Registry& registry) {
        if (!registry.valid(active_entity)) {
            active_entity = entt::null;
        }

        EntityType entity_clicked = active_entity;
        registry.each([&](auto entity) {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick
                | ImGuiTreeNodeFlags_SpanAvailWidth;
            if (entity == active_entity)
                flags |= ImGuiTreeNodeFlags_Selected;

            bool node_open = ImGui::TreeNodeEx(reinterpret_cast<const void*>((uintptr_t)entt::to_integral(entity)),
                flags, "Selectable Node for entity %d", entt::to_integral(entity));
            if (ImGui::IsItemClicked())
                entity_clicked = entity;
            if (node_open) {
                ImGui::BulletText("This node is open");
                ImGui::TreePop();
            }
        });
        if (entity_clicked != active_entity) {
            active_entity = entity_clicked;
        }
    }

    EntityType get_active() const { return active_entity; }
};
}
