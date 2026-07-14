#include "ui/widgets/asset_list.hpp"
#include "editor_layer.hpp"

#include <IconsFontAwesome5.h>
#include <algorithm>
#include <vector>

namespace k2::editor {

void AssetListWidget::render(EditorLayer& editor_layer) {
    ImGui::BeginDisabled(!editor_layer.project.has_value());
    if (ImGui::Button(ICON_FA_SYNC "  Reload")) {
        try {
            editor_layer.reload_assets();
            Log::core().info("Reloaded asset bundle.");
        } catch (const std::exception& e) {
            Log::core().error(std::format("Failed to reload assets: {}", e.what()));
        }
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

    constexpr auto flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY
        | ImGuiTableFlags_BordersInnerV;
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
            ImGui::TextUnformatted(asset_name.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(std::string { asset.get_type_strv() }.c_str());
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(asset.url.c_str());
        }
        ImGui::EndTable();
    }
}
}
