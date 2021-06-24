#pragma once

#include <cassert>
#include <functional>
#include <map>
#include <set>
#include <string>

#include "entt/entt.hpp"
#include "imgui.h"

constexpr static inline auto K2_IMGUI_PAYLOAD_TYPE_ENTITY = "k2_entity";

namespace k2 {

template <class EntityType>
inline void EntityWidget(EntityType& entity, entt::basic_registry<EntityType>& registry, bool dropTarget = false) {
    ImGui::PushID(static_cast<int>(entt::to_integral(entity)));

    if (registry.valid(entity)) {
        ImGui::Text("ID: %d", entt::to_integral(entity));
    } else {
        ImGui::Text("Invalid Entity");
    }

    if (registry.valid(entity)) {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload(K2_IMGUI_PAYLOAD_TYPE_ENTITY, &entity, sizeof(entity));
            ImGui::Text("ID: %d", entt::to_integral(entity));
            ImGui::EndDragDropSource();
        }
    }

    if (dropTarget && ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(K2_IMGUI_PAYLOAD_TYPE_ENTITY)) {
            entity = *(EntityType*)payload->Data;
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::PopID();
}

template <class Component, class EntityType>
void ComponentEditorWidget(
    [[maybe_unused]] entt::basic_registry<EntityType>& registry, [[maybe_unused]] EntityType entity) { }

template <class Component, class EntityType>
void ComponentAddAction(entt::basic_registry<EntityType>& registry, EntityType entity) {
    registry.template emplace<Component>(entity);
}

template <class Component, class EntityType>
void ComponentRemoveAction(entt::basic_registry<EntityType>& registry, EntityType entity) {
    registry.template remove<Component>(entity);
}

template <class EntityType> class EntityEditor {
public:
    using Registry = entt::basic_registry<EntityType>;
    using ComponentTypeID = entt::id_type;

    struct ComponentInfo {
        using Callback = std::function<void(Registry&, EntityType)>;
        std::string name;
        Callback widget, create, destroy;
    };

    bool show_window = true;

private:
    std::map<ComponentTypeID, ComponentInfo> component_infos;

    bool entity_has_component(Registry& registry, EntityType& entity, ComponentTypeID type_id) {
        ComponentTypeID type[] = { type_id };
        return registry.runtime_view(std::cbegin(type), std::cend(type)).contains(entity);
    }

public:
    template <class Component> ComponentInfo& register_component(const ComponentInfo& component_info) {
        auto index = entt::type_hash<Component>::value();
        auto insert_info = component_infos.insert_or_assign(index, component_info);
        assert(insert_info.second);
        return std::get<ComponentInfo>(*insert_info.first);
    }

    template <class Component>
    ComponentInfo& register_component(const std::string& name, typename ComponentInfo::Callback widget) {
        return register_component<Component>(ComponentInfo {
            name,
            widget,
            ComponentAddAction<Component, EntityType>,
            ComponentRemoveAction<Component, EntityType>,
        });
    }

    template <class Component> ComponentInfo& register_component(const std::string& name) {
        return register_component<Component>(name, ComponentEditorWidget<Component, EntityType>);
    }

    void render_editor(Registry& registry, EntityType& entity) {
        ImGui::TextUnformatted("Editing:");
        ImGui::SameLine();

        EntityWidget(entity, registry, true);

        if (ImGui::Button("New")) {
            entity = registry.create();
        }
        if (registry.valid(entity)) {
            ImGui::SameLine();

            // clone would go here
            // if (ImGui::Button("Clone")) {
            // auto old_e = e;
            // e = registry.create();
            //}

            ImGui::Dummy({ 10, 0 }); // space destroy a bit, to not accidentally click it
            ImGui::SameLine();

            // red button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.15f, 0.15f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.3f, 0.3f, 1.f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.f, 0.2f, 0.2f, 1.f));
            if (ImGui::Button("Destroy")) {
                registry.destroy(entity);
                entity = entt::null;
            }
            ImGui::PopStyleColor(3);
        }

        ImGui::Separator();

        if (registry.valid(entity)) {
            ImGui::PushID(static_cast<int>(entt::to_integral(entity)));
            std::map<ComponentTypeID, ComponentInfo> has_not;
            for (auto& [component_type_id, ci] : component_infos) {
                if (entity_has_component(registry, entity, component_type_id)) {
                    ImGui::PushID(component_type_id);
                    if (ImGui::Button("-")) {
                        ci.destroy(registry, entity);
                        ImGui::PopID();
                        continue; // early out to prevent access to deleted data
                    } else {
                        ImGui::SameLine();
                    }

                    if (ImGui::CollapsingHeader(ci.name.c_str())) {
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

            if (!has_not.empty()) {
                if (ImGui::Button("+ Add Component")) {
                    ImGui::OpenPopup("Add Component");
                }

                if (ImGui::BeginPopup("Add Component")) {
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

    void render_entity_list(Registry& registry, std::set<ComponentTypeID>& components) {
        ImGui::Text("Components Filter:");
        ImGui::SameLine();
        if (ImGui::SmallButton("clear")) {
            components.clear();
        }

        ImGui::Indent();

        for (const auto& [component_type_id, ci] : component_infos) {
            bool is_in_list = components.count(component_type_id);
            bool active = is_in_list;

            ImGui::Checkbox(ci.name.c_str(), &active);

            if (is_in_list && !active) { // remove
                components.erase(component_type_id);
            } else if (!is_in_list && active) { // add
                components.emplace(component_type_id);
            }
        }

        ImGui::Unindent();
        ImGui::Separator();

        if (components.empty()) {
            ImGui::Text("Orphans:");
            registry.orphans([&registry](auto e) { k2::EntityWidget(e, registry, false); });
        } else {
            auto view = registry.runtime_view(components.begin(), components.end());
            ImGui::Text("%lu Entities Matching:", view.size_hint());

            if (ImGui::BeginChild("entity list")) {
                for (auto e : view) {
                    k2::EntityWidget(e, registry, false);
                }
            }
            ImGui::EndChild();
        }
    }

    [[deprecated("Use render_editor() instead. And manage the window yourself.")]] void render(
        Registry& registry, EntityType& e) {
        if (show_window) {
            if (ImGui::Begin("Entity Editor", &show_window)) {
                render_editor(registry, e);
            }
            ImGui::End();
        }
    }

    // displays both, editor and list
    // uses static internally, use only as a quick way to get going!
    void render_simple_combo(Registry& registry, EntityType& e) {
        if (show_window) {
            ImGui::SetNextWindowSize(ImVec2(550, 400), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Entity Editor", &show_window)) {
                if (ImGui::BeginChild("list", { 200, 0 }, true)) {
                    static std::set<ComponentTypeID> comp_list;
                    render_entity_list(registry, comp_list);
                }
                ImGui::EndChild();

                ImGui::SameLine();

                if (ImGui::BeginChild("editor")) {
                    render_editor(registry, e);
                }
                ImGui::EndChild();
            }
            ImGui::End();
        }
    }
};
}