#pragma once
#include "editor_resources.hpp"
#include <IconsFontAwesome5.h>
#include <filesystem>
#include <imgui.h>

namespace k2::editor {
class FileExplorerWidget {
    std::filesystem::path current_directory = std::filesystem::current_path();

public:
    void render() {
        namespace fs = std::filesystem;
        static auto icon_size = 75.f;
        constexpr auto icon_padding = 20.f;

        auto avail_width = ImGui::GetContentRegionAvail().x;
        auto num_columns = static_cast<int>(avail_width / (icon_size + icon_padding));

        if (ImGui::Button(ICON_FA_BACKWARD)) {
            current_directory = current_directory.parent_path();
        }
        ImGui::SameLine();
        ImGui::Text("%s", current_directory.string().c_str());
        ImGui::SameLine();
        static ImGuiTextFilter filter;
        filter.Draw(ICON_FA_SEARCH " Search", 200.0f);
        if (ImGui::BeginTable("Directory View", num_columns)) {
            std::uint64_t count {};
            for (const auto& entry : fs::directory_iterator(current_directory)) {
                std::string path = entry.path().filename().string();
                if (filter.PassFilter(path.c_str(), path.c_str() + path.size())) {
                    if (count && count % num_columns == 0) {
                        ImGui::TableNextRow();
                    }
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
                    count++;
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