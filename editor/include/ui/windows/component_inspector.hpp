#pragma once

#include "ui/widgets/component_inspector.hpp"
#include "ui/windows/window.hpp"

#include "components/camera.hpp"
#include "components/sprite.hpp"
#include "components/tag.hpp"
#include "components/transform.hpp"

namespace k2::editor {
template <class EntityType> class ComponentInspectorWindow : public IImGuiWindow {
    ComponentInspectorWidget<EntityType> widget;

public:
    explicit ComponentInspectorWindow(const std::string& title)
        : IImGuiWindow(title) { }

    auto& get_widget() { return widget; }

    void render_internal(EditorLayer& editor_layer) override { widget.render(editor_layer); }
};
}