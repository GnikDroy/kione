#pragma once
#include "ui/widgets/file_explorer.hpp"
#include "editor_layer.hpp"

namespace k2::editor {
// TODO: Optimize
// Store dirent and current_path instead of querying every frame. Add reload button.

void FileExplorerWidget::render(EditorLayer&) {
    namespace fs = std::filesystem;
    static auto icon_size = 75.f;
    constexpr auto icon_padding = 20.f;

    auto avail_width = ImGui::GetContentRegionAvail().x;
    auto num_columns = std::max(1, static_cast<int>(avail_width / (icon_size + icon_padding)));

    static ImGuiTextFilter filter;
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
        ImGui::EndTable();
    }

    if (ImGui::BeginTable("Directory View", num_columns)) {
        for (const auto& entry : fs::directory_iterator(current_directory)) {
            std::string path = entry.path().filename().string();
            if (filter.PassFilter(path.c_str(), path.c_str() + path.size())) {
                ImGui::TableNextColumn();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                auto texture_id = predict_icon_texture(entry);
                ImGui::ImageButton((const char*)entry.path().string().c_str(), (std::uint64_t)texture_id,
                    { icon_size, icon_size }, { 0, 1 }, { 1, 0 });
                ImGui::PopStyleColor();

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    if (entry.is_directory())
                        current_directory /= entry.path().filename();
                }
                ImGui::TextWrapped("%s", entry.path().filename().string().c_str());
            }
        }
        ImGui::EndTable();
    }
}

std::uint64_t FileExplorerWidget::predict_icon_type(const std::filesystem::directory_entry& dirent) {
    using namespace k2::literals;
    if (dirent.is_directory()) {
        return "icon_folder"_fnv1a;
    }
    auto&& ext = dirent.path().extension();
    if (ext == ".jpg" || ext == ".png" || ext == ".bmp" || ext == ".jpeg" || ext == ".gif") {
        return "icon_image"_fnv1a;
    }
    return "icon_file"_fnv1a;
}

ResourceID FileExplorerWidget::predict_icon_texture(const std::filesystem::directory_entry& dirent) {
    return editor::Resources::get<Texture2D>(predict_icon_type(dirent)).id;
}

}
