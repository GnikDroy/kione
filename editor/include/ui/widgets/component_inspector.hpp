#pragma once

#include "ui/widgets/widget.hpp"
#include <IconsFontAwesome5.h>
#include <cassert>
#include <entt/entt.hpp>
#include <functional>
#include <imgui.h>
#include <utility>
#include <vector>

namespace k2::editor {

template <class Component, class EntityType>
void ComponentWidget([[maybe_unused]] entt::basic_registry<EntityType>& registry, [[maybe_unused]] EntityType entity) {
}

template <class Component, class EntityType>
void ComponentAddAction(entt::basic_registry<EntityType>& registry, EntityType entity) {
    registry.template emplace<Component>(entity);
}

template <class Component, class EntityType>
void ComponentRemoveAction(entt::basic_registry<EntityType>& registry, EntityType entity) {
    registry.template remove<Component>(entity);
}

template <class Component, class EntityType>
void ComponentCopyAction(entt::basic_registry<EntityType>& registry, EntityType from, EntityType to) {
    if constexpr (std::is_empty_v<Component>) {
        if (registry.template all_of<Component>(from)) {
            registry.template emplace_or_replace<Component>(to);
        }
    } else {
        if (const auto* component = registry.template try_get<Component>(from)) {
            registry.template emplace_or_replace<Component>(to, *component);
        }
    }
}

template <class EntityType> class ComponentInspectorWidget : public IWidget {
public:
    using Registry = entt::basic_registry<EntityType>;
    using ComponentTypeID = entt::id_type;

    struct ComponentInfo {
        using Callback = std::function<void(Registry&, EntityType)>;
        using CopyCallback = std::function<void(Registry&, EntityType, EntityType)>;
        std::string name;
        Callback widget, create, destroy;
        CopyCallback copy;
    };

private:
    std::vector<std::pair<ComponentTypeID, ComponentInfo>> component_infos;

    bool entity_has_component(Registry& registry, EntityType entity, ComponentTypeID type_id) {
        const auto* storage_ptr = registry.storage(type_id);
        return storage_ptr != nullptr && storage_ptr->contains(entity);
    }

public:
    ComponentInspectorWidget();

    template <class Component> ComponentInfo& register_component(const ComponentInfo& component_info) {
        auto index = entt::type_hash<Component>::value();
        assert(std::ranges::find(component_infos, index, &std::pair<ComponentTypeID, ComponentInfo>::first)
            == component_infos.end());
        return component_infos.emplace_back(index, component_info).second;
    }

    template <class Component>
    ComponentInfo& register_component(const std::string& name, typename ComponentInfo::Callback widget) {
        return register_component<Component>(ComponentInfo {
            name,
            widget,
            ComponentAddAction<Component, EntityType>,
            ComponentRemoveAction<Component, EntityType>,
            ComponentCopyAction<Component, EntityType>,
        });
    }

    template <class Component> ComponentInfo& register_component(const std::string& name) {
        return register_component<Component>(name, ComponentWidget<Component, EntityType>);
    }

    void copy_components(Registry& registry, EntityType from, EntityType to) {
        for (auto& [component_type_id, ci] : component_infos) {
            if (ci.copy) {
                ci.copy(registry, from, to);
            }
        }
    }

    void add_component_menu_items(Registry& registry, EntityType entity) {
        for (auto& [component_type_id, ci] : component_infos) {
            if (entity_has_component(registry, entity, component_type_id)) {
                continue;
            }
            ImGui::PushID(int(component_type_id));
            if (ImGui::MenuItem(ci.name.c_str())) {
                ci.create(registry, entity);
            }
            ImGui::PopID();
        }
    }

    void render(EditorLayer&) override;
};
}
