#pragma once
#include "ui/widgets/entity_selector.hpp"
#include "ui/windows/window.hpp"

namespace k2::editor {
template <class EntityType> class EntitySelectorWindow : public IImGuiWindow {
    EntitySelector<EntityType> widget;

protected:
    void render_internal(EditorLayer& editor_layer) override { widget.render(editor_layer); }

public:
    explicit EntitySelectorWindow(const std::string& title)
        : IImGuiWindow(title) {};
    auto& get_widget() { return widget; }
};
}
