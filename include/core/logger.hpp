#pragma once
#include <memory>
#include <string>

namespace k2 {
namespace Log {
    class Logger {
        struct Impl;
        std::unique_ptr<Impl> impl;

    public:
        Logger(const std::string& name);
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        void trace(const std::string& msg);
        void info(const std::string& msg);
        void warn(const std::string& msg);
        void error(const std::string& msg);
        void critical(const std::string& msg);
    };

    Logger& app();
    Logger& core();
}
} // namespace k2
