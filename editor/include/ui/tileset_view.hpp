#pragma once
#include "rendering/texture.hpp"
#include "rendering/tileset.hpp"

#include <algorithm>
#include <cstdint>
#include <imgui.h>

namespace k2::editor {

inline int TileAtlasGrid(
    const char* id, const k2::TileSet& tileset, const k2::Texture2D& texture, int selected, float zoom = 1.0f) {
    ImGui::PushID(id);

    int cols = tileset.columns();
    int rows = tileset.rows();
    float avail = std::max(ImGui::GetContentRegionAvail().x, 64.0f) * zoom;
    float aspect = texture.width > 0 ? float(texture.height) / float(texture.width) : 1.0f;
    ImVec2 size { avail, avail * aspect };
    ImVec2 origin = ImGui::GetCursorScreenPos();

    ImGui::Image((ImTextureID)(std::uint64_t)texture.id, size, { 0, 1 }, { 1, 0 });
    bool hovered = ImGui::IsItemHovered();
    ImVec2 mouse = ImGui::GetIO().MousePos;

    float sx = texture.width > 0 ? size.x / float(texture.width) : 0.0f;
    float sy = texture.height > 0 ? size.y / float(texture.height) : 0.0f;
    auto* draw_list = ImGui::GetWindowDrawList();

    auto cell_rect = [&](int col, int row) {
        auto px = tileset.margin + glm::ivec2 { col, row } * (tileset.tile_size + tileset.spacing);
        ImVec2 min { origin.x + float(px.x) * sx, origin.y + float(px.y) * sy };
        ImVec2 max { min.x + float(tileset.tile_size.x) * sx, min.y + float(tileset.tile_size.y) * sy };
        return std::pair { min, max };
    };

    int clicked = -1;
    // A dark underlay plus a light line for grid lines.
    auto grid_shadow = ImGui::GetColorU32({ 0.0f, 0.0f, 0.0f, 0.35f });
    auto grid_color = ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 0.45f });
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            auto [min, max] = cell_rect(col, row);
            draw_list->AddRect({ min.x + 1.0f, min.y + 1.0f }, { max.x + 1.0f, max.y + 1.0f }, grid_shadow);
            draw_list->AddRect(min, max, grid_color);

            int index = row * cols + col;
            if (index == selected) {
                draw_list->AddRectFilled(min, max, ImGui::GetColorU32({ 0.25f, 0.55f, 1.0f, 0.35f }));
                draw_list->AddRect(min, max, ImGui::GetColorU32({ 0.35f, 0.65f, 1.0f, 1.0f }), 0.0f, 0, 2.0f);
            }
            if (hovered && mouse.x >= min.x && mouse.x < max.x && mouse.y >= min.y && mouse.y < max.y) {
                draw_list->AddRect(min, max, ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 0.6f }));
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                    clicked = index;
                }
            }
        }
    }
    // Brighter outer boundary
    draw_list->AddRect(origin, { origin.x + size.x, origin.y + size.y },
        ImGui::GetColorU32({ 1.0f, 1.0f, 1.0f, 0.85f }), 0.0f, 0, 1.5f);

    ImGui::PopID();
    return clicked;
}
}
