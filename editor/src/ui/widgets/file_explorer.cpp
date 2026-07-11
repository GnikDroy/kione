#include "ui/widgets/file_explorer.hpp"
#include "core/logger.hpp"
#include "editor_layer.hpp"

#include <format>
#include <system_error>

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
            cached_entries.second.push_back(*it);
        }
    }
}

void FileExplorerWidget::render(EditorLayer& editor_layer) {
    if (ImGui::BeginTable("##FileExplorerHeader", 2, ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableNextColumn();
        if (ImGui::Button(ICON_FA_BACKWARD)) {
            current_directory = current_directory.parent_path();
        }
        ImGui::SameLine();
        {
            constexpr auto max_str_size = 80;
            auto str = current_directory.string();
            auto ptr = str.c_str();
            if (str.size() > max_str_size + 3) {
                ptr = ptr + (str.size() - max_str_size - 2);
                for (auto i = 0; i < 3; i++) {
                    str[str.size() - max_str_size - i] = '.';
                }
            }
            ImGui::TextUnformatted(ptr);
        }

        ImGui::TableNextColumn();
        filter.Draw(ICON_FA_SEARCH, 200.0f);

        ImGui::SameLine();
        if (ImGui::Button(ICON_FA_SYNC)) {
            cached_entries.first.clear();
        }

        ImGui::EndTable();
    }
    render_directory_table(editor_layer.resources);
}

void FileExplorerWidget::render_directory_table(k2::ResourceManager& resources) {
    namespace fs = std::filesystem;
    auto avail_width = ImGui::GetContentRegionAvail().x;
    auto num_columns = std::max(1, static_cast<int>(avail_width / (icon_size + icon_padding)));

    if (ImGui::BeginTable("Directory View", num_columns)) {
        cache_entries();
        for (const auto& entry : cached_entries.second) {
            render_directory(resources, entry);
        }
        ImGui::EndTable();
    }
}

void FileExplorerWidget::render_directory(k2::ResourceManager& resources, const std::filesystem::directory_entry& entry) {
    std::string path = entry.path().filename().string();
    if (filter.PassFilter(path.c_str(), path.c_str() + path.size())) {
        ImGui::TableNextColumn();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        auto texture_id = predict_icon_texture(resources, entry);
        ImGui::ImageButton((const char*)entry.path().string().c_str(), (std::uint64_t)texture_id,
            { icon_size, icon_size }, { 0, 1 }, { 1, 0 });
        ImGui::PopStyleColor();

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            std::error_code ec;
            if (entry.is_directory(ec))
                current_directory /= entry.path().filename();
        }
        ImGui::TextWrapped("%s", path.c_str());
    }
}

std::uint64_t FileExplorerWidget::predict_icon_type(const std::filesystem::directory_entry& dirent) {
    using namespace k2::literals;
    std::error_code ec;
    if (dirent.is_directory(ec)) {
        return "icon_folder"_fnv1a;
    }
    auto&& ext = dirent.path().extension();
    if (ext == ".jpg" || ext == ".png" || ext == ".bmp" || ext == ".jpeg" || ext == ".gif") {
        return "icon_image"_fnv1a;
    }
    return "icon_file"_fnv1a;
}

ResourceID FileExplorerWidget::predict_icon_texture(
    k2::ResourceManager& resources, const std::filesystem::directory_entry& dirent) {
    auto* texture = resources.try_get<Texture2D>(predict_icon_type(dirent));
    return texture ? texture->id : 0;
}

}
