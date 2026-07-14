#pragma once

#include "ui/widgets/asset_list.hpp"
#include "ui/windows/window.hpp"

namespace k2::editor {
class AssetListWindow : public IImGuiWindow {
    AssetListWidget widget;

protected:
    void render_internal(EditorLayer& editor_layer) override { widget.render(editor_layer); }

public:
    explicit AssetListWindow(const std::string& title)
        : IImGuiWindow(title) { }
};

}
