#pragma once

#include "ui/widgets/log_viewer.hpp"
#include "editor_layer.hpp"

namespace k2::editor {

void LogViewer::render(EditorLayer& editor_layer) {
    auto* theme = editor_layer.theme.get();

    auto&& lines = EditorLoggerSink::get().get(EditorLoggerSink::max_items);
    static ImGuiTextFilter filter;
    filter.Draw(ICON_FA_SEARCH " Search");
    for (const auto& [log_level, line] : lines) {
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
            ImGui::TextColored(theme->colors.at(hash), "%s", line.c_str());
        }
    }
}
}