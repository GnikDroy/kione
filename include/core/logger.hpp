#pragma once
#include <any>
#include <memory>
#include <string>
#include <vector>

namespace k2 {
namespace Log {
    enum class LogLevel {
        Trace,
        Debug,
        Info,
        Warn,
        Err,
        Critical,
        Off,
    };

    class Logger {
        struct Impl;
        std::unique_ptr<Impl> impl;

    public:
        Logger(const std::string& name);
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        Logger& add_sink(std::any& sink_handle);

        Logger& set_level(LogLevel level);
        Logger& trace(const std::string& msg);
        Logger& info(const std::string& msg);
        Logger& warn(const std::string& msg);
        Logger& error(const std::string& msg);
        Logger& critical(const std::string& msg);
    };

    enum class LoggerSinkType {
        SingleThreaded,
        MultiThreaded,
    };

    template <LoggerSinkType T> class RingBufferSink {
        std::any impl;

    public:
        explicit RingBufferSink(size_t num_items = 0);
        std::any& get_sink();
        void set_pattern(const std::string& pattern);
        std::vector<std::pair<LogLevel, std::string>> get(size_t num_items = 0);
    };

    Logger& app();
    Logger& core();
}
} // namespace k2
