#pragma once
#include "editor_resources.hpp"
#include <IconsFontAwesome5.h>
#include <filesystem>
#include <imgui.h>

namespace k2::editor {
// TODO: Optimize
// Store dirent and current_path instead of querying every frame. Add reload button.

class FileExplorerWidget {
    std::filesystem::path current_directory = std::filesystem::current_path();

public:
    void render() {
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
                    ImGui::ImageButton((void*)(std::uintptr_t)texture_id, { icon_size, icon_size }, { 0, 1 }, { 1, 0 });
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

private:
    std::uint64_t predict_icon_type(const std::filesystem::directory_entry& dirent) {
        using namespace k2::literals;
        if (dirent.is_directory()) {
            return "folder"_fnv1a;
        }
        auto&& ext = dirent.path().extension();
        if (ext == ".jpg" || ext == ".png" || ext == ".bmp" || ext == ".jpeg" || ext == ".gif") {
            return "image"_fnv1a;
        }
        if (ext == ".cpp" || ext == ".hpp" || ext == ".h" || ext == ".c" || ext == ".glsl") {
            return "scripts"_fnv1a;
        }
        if (ext == ".mp3" || ext == ".wav" || ext == ".ogg") {
            return "audio"_fnv1a;
        }
        if (ext == ".mp4" || ext == ".avi" || ext == ".flv") {
            return "video"_fnv1a;
        }
        if (dirent.path().filename() == "bundle.yaml") {
            return "asset_bundle"_fnv1a;
        }
        return "file"_fnv1a;
    }

    ResourceID predict_icon_texture(const std::filesystem::directory_entry& dirent) {
        return editor::Resources::get<Texture2D>(predict_icon_type(dirent)).id;
    }
};

}