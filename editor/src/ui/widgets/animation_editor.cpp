#include "ui/widgets/animation_editor.hpp"
#include "asset/loader.hpp"
#include "editor_layer.hpp"
#include "serializers/asset/sprite_animation.hpp" // IWYU pragma: keep
#include "ui/common.hpp"

#include <IconsMaterialSymbols.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <nfd.hpp>

namespace k2::editor {

void AnimationEditorWidget::load_clip(EditorLayer& editor_layer) {
    loaded_id = selected.id;
    loaded = false;
    preview_elapsed = 0.0f;

    auto it = editor_layer.active_assets().find(selected.id);
    if (it == editor_layer.active_assets().end() || it->second.second.type != Asset::Type::Animation) {
        return;
    }
    auto result = AssetLoader::try_get<SpriteAnimation>(it->second.second);
    if (!result) {
        Log::core().error(std::format("Failed to load animation clip '{}': {}", selected.name, result.error()));
        return;
    }
    clip = std::move(*result);
    loaded = true;
}

void AnimationEditorWidget::save_clip(EditorLayer& editor_layer) {
    auto it = editor_layer.active_assets().find(selected.id);
    if (it == editor_layer.active_assets().end()) {
        Log::core().error(std::format("Animation clip '{}' is not in the asset registry", selected.name));
        return;
    }
    std::filesystem::path path { it->second.second.get_url_divisions().path };
    std::ofstream out { path };
    out << YAML::Node { clip } << "\n";
    if (!out) {
        Log::core().error(std::format("Failed to save animation clip: {}", path.string()));
        return;
    }
    // Make the edit visible to preview and the next play without a reload.
    editor_layer.runtime.resources.set(selected.name, clip);
    Log::core().info(std::format("Saved animation clip: {}", path.string()));
}

void AnimationEditorWidget::new_clip(EditorLayer& editor_layer) {
    std::array filters = { nfdfilteritem_t { "Kione animation", "k2anim" } };
    [[maybe_unused]] auto lock = NFD::Guard();
    NFD::UniquePathU8 chosen;
    if (NFD::SaveDialog(chosen, filters.data(), nfdfiltersize_t(filters.size())) != NFD_OKAY) {
        return;
    }
    std::filesystem::path path { chosen.get() };
    if (path.extension() != ".k2anim") {
        path += ".k2anim";
    }

    SpriteAnimation fresh { .frames = { {} } };
    {
        std::ofstream out { path };
        out << YAML::Node { fresh } << "\n";
        if (!out) {
            Log::core().error(std::format("Failed to write animation clip: {}", path.string()));
            return;
        }
    }

    auto& project = *editor_layer.project;
    auto name = path.stem().string();
    std::error_code ec;
    auto relative = std::filesystem::relative(path, project.root, ec);
    if (ec) {
        Log::core().error(std::format("Clip path cannot be made project-relative: {}", ec.message()));
        return;
    }
    if (!project.assets_node.IsDefined() || project.assets_node.IsNull()) {
        project.assets_node = YAML::Node { YAML::NodeType::Map };
    }
    project.assets_node["Animation"][name] = std::format("file:///{}", relative.generic_string());
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
    Log::core().info(std::format("Created animation clip: {}", path.string()));
}

static int frame_index_at(const k2::SpriteAnimation& clip, float time) {
    float accumulated = 0.0f;
    for (int i = 0; i < int(clip.frames.size()); i++) {
        accumulated += std::max(clip.frames[size_t(i)].duration, 0.0f);
        if (time < accumulated) {
            return i;
        }
    }
    return int(clip.frames.size()) - 1;
}

static float frame_start_time(const k2::SpriteAnimation& clip, int index) {
    float accumulated = 0.0f;
    for (int i = 0; i < index && i < int(clip.frames.size()); i++) {
        accumulated += std::max(clip.frames[size_t(i)].duration, 0.0f);
    }
    return accumulated;
}

float AnimationEditorWidget::draw_preview(EditorLayer& editor_layer) {
    const SpriteAnimation::Frame* frame = nullptr;
    auto length = clip.length();
    auto time = 0.0f;
    if (!clip.frames.empty()) {
        if (preview_playing) {
            preview_elapsed += ImGui::GetIO().DeltaTime;
            time = preview_elapsed;
            if (length > 0.0f) {
                time = clip.loop ? std::fmod(time, length) : std::min(time, length);
            } else {
                time = 0.0f;
            }
            frame = &clip.frame_at(time);
        } else {
            time = preview_elapsed;
            frame = &clip.frames[size_t(selected_frame)];
        }
    }

    const auto* texture = frame ? editor_layer.runtime.resources.try_get<Texture2D>(clip.texture.id) : nullptr;
    if (texture != nullptr && texture->width > 0 && texture->height > 0) {
        constexpr auto stage = 128.0f;
        // region is in texture pixels; normalize to uv for the preview image.
        k2::Rectf uv { .x = frame->region.x / float(texture->width), .y = frame->region.y / float(texture->height),
            .w = frame->region.w / float(texture->width), .h = frame->region.h / float(texture->height) };
        auto pixel_width = std::abs(frame->region.w);
        auto pixel_height = std::abs(frame->region.h);
        auto size = ImVec2 { stage, stage };
        if (pixel_width > 0.0f && pixel_height > 0.0f) {
            auto scale = std::min(stage / pixel_width, stage / pixel_height);
            size = { pixel_width * scale, pixel_height * scale };
        }

        auto cursor = ImGui::GetCursorPos();
        ImGui::Dummy({ stage, stage });
        ImGui::SetCursorPos({ cursor.x + (stage - size.x) * 0.5f, cursor.y + (stage - size.y) * 0.5f });
        // Engine textures are v-flipped
        ImGui::ImageWithBg((std::uint64_t)texture->id, size, { uv.x, uv.y + uv.h }, { uv.x + uv.w, uv.y },
            { 0.0f, 0.0f, 0.0f, 0.0f }, { frame->color.r, frame->color.g, frame->color.b, frame->color.a });
    } else {
        ImGui::Dummy({ 128.0f, 128.0f });
    }
    return time;
}

void AnimationEditorWidget::draw_frame_stage() {
    auto& style = ImGui::GetStyle();
    auto button_width = [&](const char* label) { return ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f; };
    auto count = int(clip.frames.size());

    auto counter = std::format("{} / {}", selected_frame + 1, count);
    auto width = count == 0
        ? button_width(ICON_MS_ADD "  Add Frame")
        : ImGui::CalcTextSize("Frame").x + ImGui::GetFrameHeight() * 2.0f + ImGui::CalcTextSize(counter.c_str()).x
            + button_width(ICON_MS_ADD) + button_width(ICON_MS_DELETE) + style.ItemSpacing.x * 5.0f + 12.0f;
    auto pad = ImGui::GetContentRegionAvail().x - width;
    if (pad > 0.0f) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + pad);
    }

    if (clip.frames.empty()) {
        if (ImGui::Button(ICON_MS_ADD "  Add Frame")) {
            clip.frames.push_back({});
            selected_frame = 0;
        }
        return;
    }

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Frame");
    ImGui::SameLine();
    if (ImGui::ArrowButton("##prev", ImGuiDir_Left)) {
        selected_frame = (selected_frame + count - 1) % count;
        preview_playing = false;
        preview_elapsed = frame_start_time(clip, selected_frame);
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(counter.c_str());
    ImGui::SameLine();
    if (ImGui::ArrowButton("##next", ImGuiDir_Right)) {
        selected_frame = (selected_frame + 1) % count;
        preview_playing = false;
        preview_elapsed = frame_start_time(clip, selected_frame);
    }

    ImGui::SameLine(0.0f, 12.0f);
    if (ImGui::Button(ICON_MS_ADD)) {
        clip.frames.insert(clip.frames.begin() + selected_frame + 1, clip.frames[size_t(selected_frame)]);
        selected_frame++;
        preview_elapsed = frame_start_time(clip, selected_frame);
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Add frame (copy of current)");
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_DELETE)) {
        clip.frames.erase(clip.frames.begin() + selected_frame);
        selected_frame = std::min(selected_frame, int(clip.frames.size()) - 1);
        preview_elapsed = selected_frame >= 0 ? frame_start_time(clip, selected_frame) : 0.0f;
    }
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Delete frame");
    }
    if (clip.frames.empty()) {
        return;
    }
}

void AnimationEditorWidget::render(EditorLayer& editor_layer) {
    ImGui::SetNextItemWidth(220.0f);
    ResourceInputWidget("##Clip", selected, editor_layer.active_assets(), Asset::Type::Animation);
    RightAlignAccentButtons({ ICON_MS_ADD "##new_clip", ICON_MS_SAVE "##save_clip" });
    if (AccentButton(ICON_MS_ADD "##new_clip", editor_layer.theme->color("primary"),
            editor_layer.project.has_value(), "New Clip")) {
        new_clip(editor_layer);
    }
    ImGui::SameLine();
    if (AccentButton(ICON_MS_SAVE "##save_clip", editor_layer.theme->color("safe"), loaded, "Save")) {
        save_clip(editor_layer);
    }

    if (selected.name.empty()) {
        ImGui::TextDisabled("Select or create an animation clip.");
        loaded = false;
        loaded_id = {};
        return;
    }
    if (selected.id != loaded_id) {
        load_clip(editor_layer);
    }
    if (!loaded) {
        ImGui::TextDisabled("Clip failed to load.");
        return;
    }

    selected_frame = std::clamp(selected_frame, 0, std::max(0, int(clip.frames.size()) - 1));

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Texture");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180.0f);
    ResourceInputWidget("##Texture", clip.texture, editor_layer.active_assets(), Asset::Type::Image);
    ImGui::SameLine(0.0f, 12.0f);
    ImGui::Checkbox("Loop", &clip.loop);

    ImGui::BeginGroup();
    auto time = draw_preview(editor_layer);
    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 16.0f);
    ImGui::BeginGroup();
    if (!clip.frames.empty()) {
        auto& frame = clip.frames[size_t(selected_frame)];
        RectField("##Region", frame.region, { .x = 0.0f, .y = 0.0f, .w = 64.0f, .h = 64.0f }, 1.0f);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::DragFloat("##Duration", &frame.duration, 0.01f, 0.0f, 60.0f, "%.3f s");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::ColorEdit4("##Color", &frame.color.x);
    }
    ImGui::EndGroup();

    if (ImGui::Button(preview_playing ? ICON_MS_PAUSE : ICON_MS_PLAY_ARROW)) {
        preview_playing = !preview_playing;
        if (!preview_playing && !clip.frames.empty()) {
            preview_elapsed = time;
            selected_frame = frame_index_at(clip, time);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_MS_UNDO)) {
        preview_elapsed = 0.0f;
        selected_frame = 0;
    }
    ImGui::SameLine();
    ImGui::Text("%.2f/%.2fs", time, clip.length());
    ImGui::SameLine();
    draw_frame_stage();
}
}
