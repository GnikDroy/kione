
#include "ui/widgets/log_viewer.hpp"
#include "editor_layer.hpp"

namespace k2::editor {

void LogViewer::render(EditorLayer& editor_layer) {
    auto* theme = editor_layer.theme.get();

    if (ImGui::Button(ICON_FA_TRASH "  Clear")) {
        EditorLoggerSink::get().clear();
    }
    ImGui::SameLine();
    constexpr std::array levels { "Trace", "Debug", "Info", "Warn", "Error", "Critical" };
    ImGui::SetNextItemWidth(110.0f);
    ImGui::Combo("##MinLevel", &min_level, levels.data(), int(levels.size()));
    ImGui::SameLine();
    filter.Draw(ICON_FA_SEARCH " Search");

    if (!ImGui::BeginChild("##LogLines")) {
        ImGui::EndChild();
        return;
    }
    auto&& lines = EditorLoggerSink::get().get(EditorLoggerSink::max_items);
    for (const auto& [log_level, line] : lines) {
        if (int(log_level) < min_level) {
            continue;
        }
        auto hash = [&] {
            using namespace k2::literals;
            switch (log_level) {
            case Log::LogLevel::Trace: return "log_trace"_fnv1a;
            case Log::LogLevel::Debug: return "log_debug"_fnv1a;
            case Log::LogLevel::Info: return "log_info"_fnv1a;
            case Log::LogLevel::Warn: return "log_warn"_fnv1a;
            case Log::LogLevel::Err: return "log_err"_fnv1a;
            case Log::LogLevel::Critical: return "log_critical"_fnv1a;
            case Log::LogLevel::Off:
            default: return "log_off"_fnv1a;
            }
        }();
        if (filter.PassFilter(line.c_str(), line.c_str() + line.size())) {
            auto color = theme->colors.find(hash);
            ImGui::TextColored(color != theme->colors.end() ? color->second : ImGui::GetStyleColorVec4(ImGuiCol_Text),
                "%s", line.c_str());
        }
    }
    // Follow new output unless the user has scrolled up.
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
}
}
