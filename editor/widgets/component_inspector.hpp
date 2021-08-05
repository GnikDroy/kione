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

template <class Component, class EntityType>
void ComponentInspectorWidget(
    [[maybe_unused]] entt::basic_registry<EntityType>& registry, [[maybe_unused]] EntityType entity) { }

template <class Component, class EntityType>
void ComponentAddAction(entt::basic_registry<EntityType>& registry, EntityType entity) {
    registry.template emplace<Component>(entity);
}

template <class Component, class EntityType>
void ComponentRemoveAction(entt::basic_registry<EntityType>& registry, EntityType entity) {
    registry.template remove<Component>(entity);
}

template <class EntityType> class ComponentInspector {
public:
    using Registry = entt::basic_registry<EntityType>;
    using ComponentTypeID = entt::id_type;

    struct ComponentInfo {
        using Callback = std::function<void(Registry&, EntityType)>;
        std::string name;
        Callback widget, create, destroy;
    };

private:
    std::map<ComponentTypeID, ComponentInfo> component_infos;

    bool entity_has_component(Registry& registry, EntityType entity, ComponentTypeID type_id) {
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
        return register_component<Component>(name, ComponentInspectorWidget<Component, EntityType>);
    }

    void render(Registry& registry, EntityType entity) {
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
};
}