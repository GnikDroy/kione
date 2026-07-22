#include "ui/widgets/tileset_editor.hpp"
#include "asset/loader.hpp"
#include "editor_layer.hpp"
#include "imgui.h"
#include "serializers/asset/tileset.hpp" // IWYU pragma: keep
#include "ui/common.hpp"
#include "ui/tileset_view.hpp"

#include <IconsMaterialSymbols.h>
#include <fstream>
#include <nfd.hpp>

namespace k2::editor {

void TileSetEditorWidget::load_tileset(EditorLayer& editor_layer) {
    loaded_id = selected.id;
    loaded = false;

    auto it = editor_layer.active_assets().find(selected.id);
    if (it == editor_layer.active_assets().end() || it->second.second.type != Asset::Type::TileSet) {
        return;
    }
    auto result = AssetLoader::try_get<TileSet>(it->second.second);
    if (!result) {
        Log::core().error(std::format("Failed to load tileset '{}': {}", selected.name, result.error()));
        return;
    }
    tileset = std::move(*result);
    loaded = true;
}

void TileSetEditorWidget::save_tileset(EditorLayer& editor_layer) {
    auto it = editor_layer.active_assets().find(selected.id);
    if (it == editor_layer.active_assets().end()) {
        Log::core().error(std::format("Tileset '{}' is not in the asset registry", selected.name));
        return;
    }
    std::filesystem::path path { it->second.second.get_url_divisions().path };
    std::ofstream out { path };
    out << YAML::Node { tileset } << "\n";
    if (!out) {
        Log::core().error(std::format("Failed to save tileset: {}", path.string()));
        return;
    }
    // Make the edit visible to any map using it without a scene reload.
    editor_layer.runtime.resources.set(selected.name, tileset);
    Log::core().info(std::format("Saved tileset: {}", path.string()));
}

void TileSetEditorWidget::new_tileset(EditorLayer& editor_layer) {
    std::array filters = { nfdfilteritem_t { .name = "Kione tileset", .spec = "k2tileset" } };
    [[maybe_unused]] auto lock = NFD::Guard();
    NFD::UniquePathU8 chosen;
    if (NFD::SaveDialog(chosen, filters.data(), nfdfiltersize_t(filters.size())) != NFD_OKAY) {
        return;
    }
    std::filesystem::path path { chosen.get() };
    if (path.extension() != ".k2tileset") {
        path += ".k2tileset";
    }

    TileSet fresh {};
    {
        std::ofstream out { path };
        out << YAML::Node { fresh } << "\n";
        if (!out) {
            Log::core().error(std::format("Failed to write tileset: {}", path.string()));
            return;
        }
    }

    auto& project = *editor_layer.project;
    auto name = path.stem().string();
    std::error_code ec;
    auto relative = std::filesystem::relative(path, project.root, ec);
    if (ec) {
        Log::core().error(std::format("Tileset path cannot be made project-relative: {}", ec.message()));
        return;
    }
    if (!project.assets_node.IsDefined() || project.assets_node.IsNull()) {
        project.assets_node = YAML::Node { YAML::NodeType::Map };
    }
    project.assets_node["TileSet"][name] = std::format("file:///{}", relative.generic_string());
    if (auto saved = project.save(); !saved) {
        Log::core().error(std::format("Failed to save project: {}", saved.error()));
        return;
    }
    if (auto reloaded = editor_layer.reload_assets(); !reloaded) {
        Log::core().error(std::format("Failed to reload assets: {}", reloaded.error()));
        return;
    }

    selected.set(name);
    loaded_id = {};
    Log::core().info(std::format("Created tileset: {}", path.string()));
}

void TileSetEditorWidget::render(EditorLayer& editor_layer) {
    ImGui::SetNextItemWidth(220.0f);
    ResourceInputWidget("##TileSet", selected, editor_layer.active_assets(), Asset::Type::TileSet);
    RightAlignAccentButtons({ ICON_MS_ADD "##new_tileset", ICON_MS_SAVE "##save_tileset" });
    if (AccentButton(ICON_MS_ADD "##new_tileset", editor_layer.theme->color("primary"),
            editor_layer.project.has_value(), "New Tile Set")) {
        new_tileset(editor_layer);
    }
    ImGui::SameLine();
    if (AccentButton(ICON_MS_SAVE "##save_tileset", editor_layer.theme->color("safe"), loaded, "Save")) {
        save_tileset(editor_layer);
    }

    if (selected.name.empty()) {
        ImGui::TextDisabled("Select or create a tile set.");
        loaded = false;
        loaded_id = {};
        return;
    }
    if (selected.id != loaded_id) {
        load_tileset(editor_layer);
    }
    if (!loaded) {
        ImGui::TextDisabled("Tile set failed to load.");
        return;
    }

    // Two label+field pairs per row: Texture | Tile Size, then Margin | Spacing.
    if (ImGui::BeginTable("TileSetFields", 4)) {
        ImGui::TableSetupColumn("##l1", ImGuiTableColumnFlags_WidthFixed, 72.0f);
        ImGui::TableSetupColumn("##f1", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("##l2", ImGuiTableColumnFlags_WidthFixed, 72.0f);
        ImGui::TableSetupColumn("##f2", ImGuiTableColumnFlags_WidthStretch, 1.0f);

        auto field_label = [](const char* text) {
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(text);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
        };

        ImGui::TableNextRow();
        field_label("Texture");
        ResourceInputWidget("##Texture", tileset.texture, editor_layer.active_assets(), Asset::Type::Image, false);
        field_label("Tile Size");
        Vec2Field("##TileSize", tileset.tile_size, { 32, 32 }, 1.0f);

        ImGui::TableNextRow();
        field_label("Margin");
        Vec2Field("##Margin", tileset.margin, { 0, 0 }, 1.0f);
        field_label("Spacing");
        Vec2Field("##Spacing", tileset.spacing, { 0, 0 }, 1.0f);

        ImGui::EndTable();
    }

    const auto* texture = editor_layer.runtime.resources.try_get<Texture2D>(tileset.texture.id);
    if (texture == nullptr || texture->width <= 0 || texture->height <= 0) {
        ImGui::TextDisabled("Texture not loaded — pick a texture to preview the atlas.");
        return;
    }
    // texture_size is derived (never serialized)
    tileset.texture_size = { texture->width, texture->height };
    ImGui::Text("%d x %d px  •  %d x %d tiles (%d)", texture->width, texture->height, tileset.columns(), tileset.rows(),
        tileset.size());
    TileAtlasGrid("##atlas", tileset, *texture, -1);
}
}
