#include "ui/widgets/asset_list.hpp"
#include "editor_layer.hpp"

#include <IconsFontAwesome5.h>
#include <algorithm>
#include <array>
#include <cctype>
#include <filesystem>
#include <optional>
#include <vector>

#include <imgui_stdlib.h>
#include <nfd.hpp>

namespace k2::editor {
namespace {

    std::optional<Asset::Type> infer_asset_type(const std::filesystem::path& file) {
        auto ext = file.extension().string();
        std::ranges::transform(ext, ext.begin(), [](unsigned char c) { return char(std::tolower(c)); });
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp") {
            return Asset::Type::Image;
        }
        if (ext == ".ttf" || ext == ".otf") {
            return Asset::Type::Font;
        }
        if (ext == ".wav" || ext == ".mp3" || ext == ".flac" || ext == ".ogg") {
            return Asset::Type::Audio;
        }
        if (ext == ".lua") {
            return Asset::Type::Script;
        }
        if (ext == ".glsl") {
            return Asset::Type::Shader;
        }
        if (ext == ".k2scene") {
            return Asset::Type::Scene;
        }
        if (ext == ".k2anim") {
            return Asset::Type::Animation;
        }
        if (ext == ".yaml") {
            return Asset::Type::AssetBundle;
        }
        return std::nullopt;
    }

    void add_asset_dialog(EditorLayer& editor_layer) {
        std::array filters = { nfdfilteritem_t { "Assets",
            "png,jpg,jpeg,tga,bmp,ttf,otf,wav,mp3,flac,ogg,lua,glsl,k2scene,k2anim,yaml" } };
        [[maybe_unused]] auto lock = NFD::Guard();
        NFD::UniquePathU8 path;
        auto root = editor_layer.project->root.string();
        if (NFD::OpenDialog(path, filters.data(), nfdfiltersize_t(filters.size()), root.c_str()) != NFD_OKAY) {
            return;
        }
        std::filesystem::path file { path.get() };
        auto type = infer_asset_type(file);
        if (!type) {
            Log::core().error(std::format("Unrecognized asset type for '{}'.", file.filename().string()));
            return;
        }
        auto name = file.stem().string();
        if (auto added = editor_layer.project->add_asset(*type, name, file); !added) {
            Log::core().error(std::format("Failed to add asset: {}", added.error()));
            return;
        }
        if (auto reloaded = editor_layer.reload_assets(); !reloaded) {
            Log::core().error(std::format("Failed to reload assets: {}", reloaded.error()));
        }
    }

}

void AssetListWidget::render(EditorLayer& editor_layer) {
    bool actionable = editor_layer.project.has_value() && !editor_layer.is_playing();

    ImGui::BeginDisabled(!editor_layer.project.has_value());
    if (ImGui::Button(ICON_FA_SYNC "  Reload")) {
        if (auto reloaded = editor_layer.reload_assets()) {
            Log::core().info("Reloaded asset manifest.");
        } else {
            Log::core().error(std::format("Failed to reload assets: {}", reloaded.error()));
        }
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!actionable);
    if (ImGui::Button(ICON_FA_PLUS "  Add")) {
        add_asset_dialog(editor_layer);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    filter.Draw(ICON_FA_SEARCH " Search", 200.0f);

    const auto& assets = editor_layer.active_assets();
    ImGui::SameLine();
    ImGui::TextDisabled("%zu assets", assets.size());

    std::vector<const std::pair<std::string, Asset>*> rows;
    rows.reserve(assets.size());
    for (const auto& [id, pair] : assets) {
        rows.push_back(&pair);
    }
    std::ranges::sort(rows, [](const auto* a, const auto* b) {
        return std::tie(a->second.type, a->first) < std::tie(b->second.type, b->first);
    });

    bool open_rename = false;
    bool open_remove = false;

    constexpr auto flags
        = ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerV;
    if (ImGui::BeginTable("##assets", 3, flags)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("URL", ImGuiTableColumnFlags_WidthStretch, 2.0f);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        for (const auto* row : rows) {
            const auto& [asset_name, asset] = *row;
            if (!filter.PassFilter(asset_name.c_str()) && !filter.PassFilter(asset.url.c_str())) {
                continue;
            }
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(asset_name.c_str());
            if (ImGui::Selectable(asset_name.c_str(), false,
                    ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)
                && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && asset.type == Asset::Type::Scene
                && !editor_layer.is_playing()) {
                if (auto opened = editor_layer.open_scene(asset_name); !opened) {
                    Log::core().error(std::format("Failed to open scene '{}': {}", asset_name, opened.error()));
                }
            }
            if (actionable && ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem(ICON_FA_PEN "  Rename")) {
                    rename_target = asset_name;
                    rename_buffer = asset_name;
                    open_rename = true;
                }
                if (ImGui::MenuItem(ICON_FA_TRASH "  Remove")) {
                    remove_target = asset_name;
                    open_remove = true;
                }
                ImGui::EndPopup();
            }
            ImGui::PopID();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(std::string { asset.get_type_strv() }.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(asset.url.c_str());
        }
        ImGui::EndTable();
    }

    if (open_rename) {
        ImGui::OpenPopup("Rename Asset");
    }
    if (open_remove) {
        ImGui::OpenPopup("Remove Asset");
    }

    if (ImGui::BeginPopupModal("Rename Asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Rename '%s' to:", rename_target.c_str());
        ImGui::SetNextItemWidth(240.0f);
        bool submit = ImGui::InputText("##NewName", &rename_buffer, ImGuiInputTextFlags_EnterReturnsTrue);
        if ((ImGui::Button("Rename") || submit) && editor_layer.project.has_value()) {
            if (auto renamed = editor_layer.project->rename_asset(rename_target, rename_buffer); !renamed) {
                Log::core().error(std::format("Failed to rename asset: {}", renamed.error()));
            } else if (auto reloaded = editor_layer.reload_assets(); !reloaded) {
                Log::core().error(std::format("Failed to reload assets: {}", reloaded.error()));
            }
            rename_target.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            rename_target.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Remove Asset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Remove '%s' from the manifest?", remove_target.c_str());
        ImGui::TextDisabled("References to it will dangle until repointed.");
        if (ImGui::Button("Remove") && editor_layer.project.has_value()) {
            if (auto removed = editor_layer.project->remove_asset(remove_target); !removed) {
                Log::core().error(std::format("Failed to remove asset: {}", removed.error()));
            } else if (auto reloaded = editor_layer.reload_assets(); !reloaded) {
                Log::core().error(std::format("Failed to reload assets: {}", reloaded.error()));
            }
            remove_target.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            remove_target.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
}
