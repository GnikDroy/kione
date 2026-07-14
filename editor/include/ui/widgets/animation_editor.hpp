#pragma once

#include "asset/asset_handle.hpp"
#include "rendering/sprite_animation.hpp"
#include "ui/widgets/widget.hpp"

namespace k2::editor {

class AnimationEditorWidget : public IWidget {
    AssetHandle selected;
    ResourceID loaded_id {};
    SpriteAnimation clip;
    bool loaded = false;

    bool preview_playing = true;
    float preview_elapsed = 0.0f;
    int selected_frame = 0;

    void load_clip(EditorLayer& editor_layer);
    void save_clip(EditorLayer& editor_layer);
    void new_clip(EditorLayer& editor_layer);
    float draw_preview(EditorLayer& editor_layer);
    void draw_frame_stage();

public:
    void render(EditorLayer&) override;
};
}
