#pragma once

#include <imgui.h>

#include "core/imgui_theme.hpp"
#include "core/logger.hpp"

namespace k2::editor {

class EditorLoggerSink {
public:
    constexpr static inline size_t max_items = 2000;

    using SinkType = k2::Log::RingBufferSink<Log::LoggerSinkType::MultiThreaded>;
    inline static SinkType& get() {
        static EditorLoggerSink instance;
        return instance.sink;
    }

private:
    SinkType sink;
    EditorLoggerSink()
        : sink(max_items) {
        Log::core().add_sink(sink.get_sink());
        Log::app().add_sink(sink.get_sink());
    }
};

class LogViewer {
public:
    void render(const Imgui::ImGuiTheme* theme) {
        auto&& lines = EditorLoggerSink::get().get(EditorLoggerSink::max_items);
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
            ImGui::TextColored(theme->colors.at(hash), "%s", line.c_str());
        }
    }
};

}