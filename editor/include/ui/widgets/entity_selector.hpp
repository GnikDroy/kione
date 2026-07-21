#pragma once

#include <entt/entt.hpp>

#include "ui/widgets/widget.hpp"

namespace k2::editor {

template <class EntityType> class EntitySelector : public IWidget {
    EntityType active_entity { entt::null };

public:
    using Registry = entt::basic_registry<EntityType>;

    void render(EditorLayer&) override;

    EntityType get_active() const { return active_entity; }

    void set_active(EntityType entity) { active_entity = entity; }

    // Must be called when the scene (registry) is replaced
    void reset_selection() { active_entity = entt::null; }
};

entt::entity duplicate_entity(EditorLayer& editor_layer, entt::registry& registry, entt::entity source);
}
