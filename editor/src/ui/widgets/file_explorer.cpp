#include "ui/widgets/file_explorer.hpp"
#include "core/logger.hpp"
#include "editor_layer.hpp"

#include <algorithm>
#include <cctype>
#include <format>
#include <system_error>

namespace {
struct IconInfo {
    const char* glyph;
    ImVec4 color;
};

IconInfo icon_for(const std::filesystem::directory_entry& entry) {
    std::error_code ec;
    if (entry.is_directory(ec)) {
        return { ICON_FA_FOLDER, { 0.85f, 0.68f, 0.39f, 1.0f } };
    }
    auto ext = entry.path().extension().string();
    std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return std::tolower(c); });
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".gif") {
        return { ICON_FA_FILE_IMAGE, { 0.78f, 0.47f, 0.87f, 1.0f } };
    }
    if (ext == ".lua") {
        return { ICON_FA_FILE_CODE, { 0.38f, 0.69f, 0.94f, 1.0f } };
    }
    if (ext == ".glsl") {
        return { ICON_FA_FILE_CODE, { 0.34f, 0.71f, 0.76f, 1.0f } };
    }
    if (ext == ".k2scene") {
        return { ICON_FA_CUBES, { 0.60f, 0.76f, 0.47f, 1.0f } };
    }
    if (ext == ".k2project") {
        return { ICON_FA_GAMEPAD, { 0.47f, 0.53f, 0.80f, 1.0f } };
    }
    if (ext == ".ttf" || ext == ".otf") {
        return { ICON_FA_FONT, { 0.90f, 0.75f, 0.48f, 1.0f } };
    }
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac") {
        return { ICON_FA_FILE_AUDIO, { 0.82f, 0.60f, 0.40f, 1.0f } };
    }
    if (ext == ".log" || ext == ".txt" || ext == ".md" || ext == ".yaml" || ext == ".yml" || ext == ".ini") {
        return { ICON_FA_FILE_ALT, { 0.62f, 0.65f, 0.71f, 1.0f } };
    }
    return { ICON_FA_FILE, { 0.54f, 0.57f, 0.62f, 1.0f } };
}
}

namespace k2::editor {

void FileExplorerWidget::cache_entries() {
    namespace fs = std::filesystem;
    if (cached_entries.first != current_directory) {
        std::error_code ec;
        auto it = fs::directory_iterator { current_directory, ec };
        if (ec) {
            Log::core().warn(
                std::format("Cannot list directory '{}': {}", current_directory.string(), ec.message()));
            current_directory = cached_entries.first;
            return;
        }
        cached_entries.first = current_directory;
        cached_entries.second.clear();
        for (; !ec && it != fs::directory_iterator {}; it.increment(ec)) {
            if (!show_hidden && it->path().filename().string().starts_with('.')) {
                continue;
            }
            cached_entries.second.push_back(*it);
        }

        std::ranges::sort(cached_entries.second, [](const auto& a, const auto& b) {
            std::error_code sort_ec;
            auto a_dir = a.is_directory(sort_ec);
            auto b_dir = b.is_directory(sort_ec);
            if (a_dir != b_dir) {
                return a_dir;
            }
            auto a_name = a.path().filename().string();
            auto b_name = b.path().filename().string();
            return std::ranges::lexicographical_compare(a_name, b_name, [](unsigned char x, unsigned char y) {
                return std::tolower(x) < std::tolower(y);
            });
        });
    }
}

void FileExplorerWidget::render_breadcrumbs() {
    namespace fs = std::filesystem;
    std::vector<fs::path> segments;
    for (auto path = current_directory;; path = path.parent_path()) {
        segments.push_back(path);
        if (!path.has_parent_path() || path == path.root_path()) {
            break;
        }
    }
    std::ranges::reverse(segments);

    constexpr std::size_t max_segments = 4;
    auto first = segments.size() > max_segments ? segments.size() - max_segments : 0;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {});
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 { 2.0f, ImGui::GetStyle().ItemSpacing.y });

    if (first > 0) {
        if (ImGui::SmallButton("...")) {
            current_directory = segments[first - 1];
        }
        ImGui::SameLine();
        ImGui::TextDisabled("/");
        ImGui::SameLine();
    }
    for (auto i = first; i < segments.size(); i++) {
        auto label = segments[i].filename().string();
        if (label.empty()) {
            label = segments[i].string();
        }
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::SmallButton(label.c_str())) {
            current_directory = segments[i];
        }
        ImGui::PopID();
        if (i + 1 < segments.size()) {
            ImGui::SameLine();
            ImGui::TextDisabled("/");
            ImGui::SameLine();
        }
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void FileExplorerWidget::render(EditorLayer& editor_layer) {
    if (ImGui::BeginTable("##FileExplorerHeader", 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableNextColumn();
        auto at_root = !current_directory.has_parent_path() || current_directory == current_directory.root_path();
        ImGui::BeginDisabled(at_root);
        if (ImGui::Button(ICON_FA_ARROW_UP)) {
            current_directory = current_directory.parent_path();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        render_breadcrumbs();

        ImGui::TableNextColumn();
        filter.Draw(ICON_FA_SEARCH, 200.0f);

        ImGui::SameLine();
        if (ImGui::Checkbox("Hidden", &show_hidden)) {
            cached_entries.first.clear();
        }

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_SYNC)) {
            cached_entries.first.clear();
        }

        ImGui::EndTable();
    }
    ImGui::Separator();
    render_directory_table(editor_layer);
}

void FileExplorerWidget::render_directory_table(EditorLayer& editor_layer) {
    auto avail_width = ImGui::GetContentRegionAvail().x;
    auto num_columns = std::max(1, static_cast<int>(avail_width / (icon_size + icon_padding)));

    if (ImGui::BeginTable("Directory View", num_columns)) {
        cache_entries();
        for (const auto& entry : cached_entries.second) {
            render_directory(editor_layer, entry);
        }
        ImGui::EndTable();
    }
}

void FileExplorerWidget::render_directory(EditorLayer& editor_layer, const std::filesystem::directory_entry& entry) {
    std::string name = entry.path().filename().string();
    if (!filter.PassFilter(name.c_str(), name.c_str() + name.size())) {
        return;
    }
    ImGui::TableNextColumn();

    auto cell_width = ImGui::GetContentRegionAvail().x;
    auto image_width = icon_size + ImGui::GetStyle().FramePadding.x * 2.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + std::max(0.0f, (cell_width - image_width) * 0.5f));

    auto path_str = entry.path().string();
    ImGui::InvisibleButton(path_str.c_str(), { icon_size, icon_size });

    auto* draw_list = ImGui::GetWindowDrawList();
    auto rect_min = ImGui::GetItemRectMin();
    auto rect_max = ImGui::GetItemRectMax();
    if (ImGui::IsItemHovered()) {
        auto highlight = ImGui::GetColorU32(ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered);
        draw_list->AddRectFilled(rect_min, rect_max, highlight, ImGui::GetStyle().FrameRounding);
    }

    auto [glyph, color] = icon_for(entry);
    auto* icon_font = editor_layer.icon_font ? editor_layer.icon_font : ImGui::GetFont();
    auto glyph_px = icon_size * 0.62f;
    auto glyph_size = icon_font->CalcTextSizeA(glyph_px, FLT_MAX, 0.0f, glyph);
    draw_list->AddText(icon_font, glyph_px,
        { rect_min.x + (icon_size - glyph_size.x) * 0.5f, rect_min.y + (icon_size - glyph_size.y) * 0.5f },
        ImGui::GetColorU32(color), glyph);

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", name.c_str());
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            std::error_code ec;
            if (entry.is_directory(ec)) {
                current_directory /= entry.path().filename();
            } else if (entry.path().extension() == ".k2project") {
                editor_layer.open_project(entry.path());
            }
        }
    }

    auto label = name;
    if (ImGui::CalcTextSize(label.c_str()).x > cell_width) {
        while (!label.empty() && ImGui::CalcTextSize((label + "...").c_str()).x > cell_width) {
            label.pop_back();
        }
        label += "...";
    }
    ImGui::SetCursorPosX(
        ImGui::GetCursorPosX() + std::max(0.0f, (cell_width - ImGui::CalcTextSize(label.c_str()).x) * 0.5f));
    ImGui::TextUnformatted(label.c_str());
    ImGui::Dummy({ 0.0f, 4.0f });
}

}
