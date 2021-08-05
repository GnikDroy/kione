#pragma once

#include <imgui.h>

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
    std::string buffer;

public:
    void render() {
        auto&& lines = EditorLoggerSink::get().get(EditorLoggerSink::max_items);

        size_t total_size {};
        for (const auto& line : lines)
            total_size += line.size();

        buffer.clear();
        buffer.reserve(total_size);
        for (const auto& line : lines)
            buffer += line;

        ImGui::TextUnformatted(buffer.data(), buffer.data() + buffer.size());
    }
};

}