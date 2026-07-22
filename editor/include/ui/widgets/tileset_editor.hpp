#pragma once

#include "asset/asset_handle.hpp"
#include "rendering/tileset.hpp"
#include "ui/widgets/widget.hpp"

namespace k2::editor {

class TileSetEditorWidget : public IWidget {
    AssetHandle selected;
    ResourceID loaded_id {};
    k2::TileSet tileset;
    bool loaded = false;

    void load_tileset(EditorLayer& editor_layer);
    void save_tileset(EditorLayer& editor_layer);
    void new_tileset(EditorLayer& editor_layer);

public:
    void render(EditorLayer&) override;
};
}
