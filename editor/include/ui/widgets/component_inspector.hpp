#pragma once

#include "ui/widgets/widget.hpp"
#include <IconsFontAwesome5.h>
#include <cassert>
#include <entt/entt.hpp>
#include <functional>
#include <map>
#include <set>
#include <string>

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

template <class EntityType> class ComponentInspectorWidget : public IWidget {
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
    ComponentInspectorWidget();

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
        return register_component<Component>(name, ComponentWidget<Component, EntityType>);
    }

    void render(EditorLayer&) override;
};
}