#pragma once

#include <IconsFontAwesome5.h>
#include <cassert>
#include <entt/entt.hpp>
#include <functional>
#include <imgui.h>
#include <map>
#include <set>
#include <string>

#include "components/tag.hpp"
#include "ui/widgets/widget.hpp"

namespace k2::editor {

template <class EntityType> class EntitySelector : public IWidget {
    EntityType active_entity { entt::null };

public:
    using Registry = entt::basic_registry<EntityType>;

    void render(EditorLayer&) override;

    EntityType get_active() const { return active_entity; }
};
}
