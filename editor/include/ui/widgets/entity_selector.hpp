#pragma once

#include <entt/entt.hpp>

#include "components/transform.hpp"
#include "ui/widgets/widget.hpp"

namespace k2::editor {

template <class EntityType> class EntitySelector : public IWidget {
    EntityType active_entity { entt::null };

public:
    using Registry = entt::basic_registry<EntityType>;

    static EntityType create_entity(Registry& registry) {
        auto entity = registry.create();
        registry.template emplace<TransformComponent>(entity);
        return entity;
    }

    void render(EditorLayer&) override;

    EntityType get_active() const { return active_entity; }

    void set_active(EntityType entity) { active_entity = entity; }

    // Must be called when the scene (registry) is replaced
    void reset_selection() { active_entity = entt::null; }
};
}
