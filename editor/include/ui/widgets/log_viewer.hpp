#pragma once

#include "core/logger.hpp"
#include "ui/widgets/widget.hpp"
#include <imgui.h>

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

class LogViewer : public IWidget {
    ImGuiTextFilter filter;
    int min_level = int(Log::LogLevel::Trace);

public:
    void render(EditorLayer&) override;
};
}
