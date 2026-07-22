#include "ui/widgets/tilemap_editor.hpp"
#include "components/transform.hpp"
#include "editor_layer.hpp"
#include "rendering/tileset.hpp"
#include "ui/tileset_view.hpp"

#include <IconsMaterialSymbols.h>
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <vector>

namespace k2::editor {

static void flood_fill(k2::TileMapComponent& tilemap, int x, int y, std::uint16_t value) {
    std::uint16_t target = tilemap[x, y];
    if (target == value) {
        return;
    }
    std::vector<glm::ivec2> stack { { x, y } };
    while (!stack.empty()) {
        auto cell = stack.back();
        stack.pop_back();
        if (!tilemap.contains(cell.x, cell.y) || tilemap[cell.x, cell.y] != target) {
            continue;
        }
        tilemap[cell.x, cell.y] = value;
        stack.emplace_back(cell.x + 1, cell.y);
        stack.emplace_back(cell.x - 1, cell.y);
        stack.emplace_back(cell.x, cell.y + 1);
        stack.emplace_back(cell.x, cell.y - 1);
    }
}

TileMapEditorWidget::TileMapEditorWidget() {
    width = 100;
    height = 100;
    resize_frame_buffer();
}

void TileMapEditorWidget::resize_frame_buffer() {
    renderer2D.set_frame_buffer({ { .width = std::size_t(width),
        .height = std::size_t(height),
        .attachments {
            {
                .buffer_type = k2::FrameBuffer::Attachment::BufferType::Texture,
                .type = k2::FrameBuffer::Attachment::Type::Color,
            },
            {
                .buffer_type = k2::FrameBuffer::Attachment::BufferType::Texture,
                .type = k2::FrameBuffer::Attachment::Type::DepthStencil,
            },
        } } });
}

void TileMapEditorWidget::update_camera() {
    renderer2D.camera = k2::Camera {
        .position { camera_position.x, camera_position.y, 1000.f },
        .target { camera_position.x, camera_position.y, 0 },
        .up { 0, 1.0f, 0 },
        .projection_traits { k2::Camera::OrthographicTraits {
            .left = -width * 0.5f * zoom,
            .right = width * 0.5f * zoom,
            .top = height * 0.5f * zoom,
            .bottom = -height * 0.5f * zoom,
            .far_clip = 0.f,
            .near_clip = 2000.f,
        } },
    };
}

void TileMapEditorWidget::push_grid_overlay(const k2::TileMapComponent& tilemap, ImVec2 rect_min) {
    if (tilemap.size.x <= 0 || tilemap.size.y <= 0) {
        return;
    }
    // Drawn as ImGui overlay, so grid lines keep a constant thickness.
    auto view = viewport(rect_min);
    auto world_to_screen = [&](glm::vec2 w) {
        auto s = renderer2D.camera.world_to_screen(w, view);
        return ImVec2 { s.x, s.y };
    };
    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->PushClipRect(rect_min, { rect_min.x + width, rect_min.y + height }, true);

    float map_w = float(tilemap.size.x) * tilemap.tile_size.x;
    float map_h = float(tilemap.size.y) * tilemap.tile_size.y;
    float cell_px = tilemap.tile_size.x / zoom; // approximate on-screen size of a cell

    // A dark underlay plus a light line for grid lines.
    auto shadow = ImGui::GetColorU32(ImVec4 { 0.0f, 0.0f, 0.0f, 0.35f });
    auto offset = [](ImVec2 p) { return ImVec2 { p.x + 1.0f, p.y + 1.0f }; };

    // Interior lines fade once cells are big enough to read.
    if (cell_px >= 4.0f) {
        auto line = ImGui::GetColorU32(ImVec4 { 1.0f, 1.0f, 1.0f, 0.45f });
        for (int col = 1; col < tilemap.size.x; col++) {
            float x = float(col) * tilemap.tile_size.x;
            auto a = world_to_screen({ x, 0.0f });
            auto b = world_to_screen({ x, -map_h });
            draw_list->AddLine(offset(a), offset(b), shadow);
            draw_list->AddLine(a, b, line);
        }
        for (int row = 1; row < tilemap.size.y; row++) {
            float y = -float(row) * tilemap.tile_size.y;
            auto a = world_to_screen({ 0.0f, y });
            auto b = world_to_screen({ map_w, y });
            draw_list->AddLine(offset(a), offset(b), shadow);
            draw_list->AddLine(a, b, line);
        }
    }
    auto tl = world_to_screen({ 0.0f, 0.0f });
    auto tr = world_to_screen({ map_w, 0.0f });
    auto br = world_to_screen({ map_w, -map_h });
    auto bl = world_to_screen({ 0.0f, -map_h });
    draw_list->AddQuad(offset(tl), offset(tr), offset(br), offset(bl), shadow, 1.5f);
    draw_list->AddQuad(tl, tr, br, bl, ImGui::GetColorU32(ImVec4 { 1.0f, 1.0f, 1.0f, 0.85f }), 1.5f);

    // Highlight the hovered cell.
    if (ImGui::IsItemHovered()) {
        auto mouse = ImGui::GetIO().MousePos;
        auto world = renderer2D.camera.screen_to_world({ mouse.x, mouse.y }, view);
        int col = int(std::floor(world.x / tilemap.tile_size.x));
        int row = int(std::floor(-world.y / tilemap.tile_size.y));
        if (tilemap.contains(col, row)) {
            float left = float(col) * tilemap.tile_size.x;
            float top = -float(row) * tilemap.tile_size.y;
            float right = left + tilemap.tile_size.x;
            float bottom = top - tilemap.tile_size.y;
            ImVec4 rgba = tool == Tool::Eraser ? ImVec4 { 1.0f, 0.4f, 0.4f, 1.0f } : ImVec4 { 0.4f, 0.7f, 1.0f, 1.0f };
            auto tl = world_to_screen({ left, top });
            auto tr = world_to_screen({ right, top });
            auto br = world_to_screen({ right, bottom });
            auto bl = world_to_screen({ left, bottom });
            draw_list->AddQuadFilled(tl, tr, br, bl, ImGui::GetColorU32(ImVec4 { rgba.x, rgba.y, rgba.z, 0.20f }));
            draw_list->AddQuad(tl, tr, br, bl, ImGui::GetColorU32(rgba), 2.0f);
        }
    }
    draw_list->PopClipRect();
}

void TileMapEditorWidget::handle_paint(k2::TileMapComponent& tilemap, ImVec2 rect_min) {
    auto& io = ImGui::GetIO();
    auto view = viewport(rect_min);
    glm::vec2 mouse { io.MousePos.x, io.MousePos.y };
    bool hovered = ImGui::IsItemHovered();

    bool panning = hovered
        && (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)
            || (ImGui::IsKeyDown(ImGuiKey_Space) && ImGui::IsMouseDragging(ImGuiMouseButton_Left)));
    if (panning) {
        camera_position.x -= io.MouseDelta.x * zoom;
        camera_position.y += io.MouseDelta.y * zoom;
        update_camera();
        return;
    }

    if (hovered && io.MouseWheel != 0.0f) {
        auto world_before = renderer2D.camera.screen_to_world(mouse, view);
        zoom = std::clamp(zoom * std::pow(0.9f, io.MouseWheel), 0.01f, 100.0f);
        update_camera();
        auto world_after = renderer2D.camera.screen_to_world(mouse, view);
        camera_position += world_before - world_after;
        update_camera();
    }

    if (!hovered || ImGui::IsKeyDown(ImGuiKey_Space)) {
        return;
    }
    auto world = renderer2D.camera.screen_to_world(mouse, view);
    int col = int(std::floor(world.x / tilemap.tile_size.x));
    int row = int(std::floor(-world.y / tilemap.tile_size.y));
    if (!tilemap.contains(col, row)) {
        return;
    }
    constexpr auto empty = k2::TileMapComponent::empty_tile;

    if (tool == Tool::Fill) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            flood_fill(tilemap, col, row, std::uint16_t(active_tile));
        } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            flood_fill(tilemap, col, row, empty);
        }
        return;
    }

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
        tilemap[col, row] = tool == Tool::Eraser ? empty : std::uint16_t(active_tile);
    } else if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
        tilemap[col, row] = empty;
    }
}

void TileMapEditorWidget::draw_canvas(EditorLayer& editor_layer, entt::entity active, k2::TileMapComponent& tilemap) {
    auto space = ImGui::GetContentRegionAvail();
    space.x = std::max(1.0f, space.x);
    space.y = std::max(1.0f, space.y);
    if (space.x != width || space.y != height) {
        width = space.x;
        height = space.y;
        resize_frame_buffer();
    }

    if (framed_for != active) {
        framed_for = active;
        float map_w = float(tilemap.size.x) * tilemap.tile_size.x;
        float map_h = float(tilemap.size.y) * tilemap.tile_size.y;
        camera_position = { map_w * 0.5f, -map_h * 0.5f };
        zoom = map_w > 0.0f && map_h > 0.0f ? std::max(map_w / width, map_h / height) * 1.15f : 1.0f;
    }

    update_camera();

    // Render the live map through a scratch one-entity scene.
    if (!canvas.registry.ctx().contains<ResourceManager&>()) {
        canvas.registry.ctx().emplace<ResourceManager&>(editor_layer.runtime.resources);
    }
    if (!canvas.registry.valid(canvas_entity)) {
        canvas_entity = canvas.registry.create();
    }
    canvas.registry.emplace_or_replace<k2::TransformComponent>(canvas_entity);
    auto preview = tilemap;
    preview.unlit = true;
    canvas.registry.emplace_or_replace<k2::TileMapComponent>(canvas_entity, std::move(preview));

    renderer2D.set_clear_color(0.13f, 0.13f, 0.15f, 1.0f);
    renderer2D.clear();
    renderer2D.draw(canvas);
    renderer2D.render();

    std::uint32_t texture_id = renderer2D.get_frame_buffer().get_traits().attachments.front().id;
    ImGui::Image((ImTextureID)(std::uint64_t)texture_id, { width, height }, { 0, 1 }, { 1, 0 });

    auto rect_min = ImGui::GetItemRectMin();
    handle_paint(tilemap, rect_min);
    push_grid_overlay(tilemap, rect_min);
}

void TileMapEditorWidget::draw_side_panel(EditorLayer& editor_layer, k2::TileMapComponent& tilemap) {
    ImGui::TextUnformatted("Tools");
    if (ImGui::Selectable(ICON_MS_BRUSH "  Brush", tool == Tool::Brush)) {
        tool = Tool::Brush;
    }
    if (ImGui::Selectable(ICON_MS_INK_ERASER "  Eraser", tool == Tool::Eraser)) {
        tool = Tool::Eraser;
    }
    if (ImGui::Selectable(ICON_MS_FORMAT_COLOR_FILL "  Fill", tool == Tool::Fill)) {
        tool = Tool::Fill;
    }

    ImGui::SeparatorText("Grid");
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::DragInt2("##size", &pending_size.x, 0.2f, 0, 4096);
    bool empty_map = tilemap.size.x <= 0 || tilemap.size.y <= 0;
    if (ImGui::Button(empty_map ? "Create Grid" : "Resize Grid", { -FLT_MIN, 0 })) {
        tilemap.resize(pending_size);
        framed_for = entt::null; // re-frame to fit the new dimensions
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Keeps existing tiles anchored at the top-left.");
    }
    ImGui::TextDisabled("Current: %d x %d", tilemap.size.x, tilemap.size.y);

    ImGui::SeparatorText("Tiles");
    auto* tileset = editor_layer.runtime.resources.try_get<TileSet>(tilemap.tileset.id);
    if (tileset == nullptr) {
        ImGui::TextWrapped("Assign a tile set to this map in the Inspector, then reopen the scene so it loads.");
        return;
    }
    auto* texture = editor_layer.runtime.resources.try_get<Texture2D>(tileset->texture.id);
    if (texture == nullptr || texture->width <= 0 || texture->height <= 0) {
        ImGui::TextWrapped("The tile set's texture is not loaded.");
        return;
    }
    if (tileset->texture_size.x <= 0 || tileset->texture_size.y <= 0) {
        tileset->texture_size = { texture->width, texture->height };
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::SliderFloat("##palette_zoom", &palette_zoom, 1.0f, 8.0f, "zoom %.1fx");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Ctrl+scroll over the palette to zoom");
    }

    ImGui::BeginChild("##palette_scroll", { 0, 0 }, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
    auto& io = ImGui::GetIO();
    if (ImGui::IsWindowHovered() && io.KeyCtrl && io.MouseWheel != 0.0f) {
        palette_zoom = std::clamp(palette_zoom * std::pow(1.1f, io.MouseWheel), 1.0f, 8.0f);
    }
    int selected = tool == Tool::Eraser ? -1 : active_tile;
    int clicked = TileAtlasGrid("##palette", *tileset, *texture, selected, palette_zoom);
    if (clicked >= 0) {
        active_tile = clicked;
        if (tool == Tool::Eraser) {
            tool = Tool::Brush;
        }
    }
    ImGui::EndChild();
}

void TileMapEditorWidget::render(EditorLayer& editor_layer) {
    auto& registry = editor_layer.active_scene().registry;
    auto active = editor_layer.entity_selector.get_widget().get_active();
    auto* tilemap = registry.valid(active) ? registry.try_get<k2::TileMapComponent>(active) : nullptr;
    if (tilemap == nullptr) {
        ImGui::TextDisabled("Select an entity with a Tile Map component to paint.");
        return;
    }

    constexpr float panel_width = 240.0f;
    auto avail = ImGui::GetContentRegionAvail();
    float canvas_width = std::max(avail.x - panel_width - ImGui::GetStyle().ItemSpacing.x, 64.0f);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
    ImGui::BeginChild("##canvas", { canvas_width, avail.y }, ImGuiChildFlags_None,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    draw_canvas(editor_layer, active, *tilemap);
    ImGui::EndChild();
    ImGui::PopStyleVar();

    ImGui::SameLine();
    ImGui::BeginChild("##panel", { 0, avail.y });
    draw_side_panel(editor_layer, *tilemap);
    ImGui::EndChild();
}
}
