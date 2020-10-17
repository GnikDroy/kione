#include "core/logger.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "fmt/core.h"

namespace k2
{
    struct Logger::Impl
    {

        Impl(const std::string &name)
        {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(fmt::format("{}.log", name), true);

            console_sink->set_pattern("%^[%T] %n: %v%$");
            file_sink->set_pattern("[%T] [%l] %n: %v");
            std::array<spdlog::sink_ptr, 2> sink_arr = {std::move(console_sink), std::move(file_sink)};

            logger = std::make_shared<spdlog::logger>(name, sink_arr.begin(), sink_arr.end());
            logger->set_level(spdlog::level::trace);
            logger->flush_on(spdlog::level::trace);
            spdlog::register_logger(logger);
        }
        std::shared_ptr<spdlog::logger> logger;
    };

    Logger::Logger(const std::string &name) : impl(std::move(std::make_unique<Logger::Impl>(name))) {}

    decltype(Logger::core) Logger::core = std::make_unique<Logger>("CORE");
    decltype(Logger::app) Logger::app = std::make_unique<Logger>("APP");

    void Logger::trace(const std::string &message) { impl->logger->trace(message); }
    void Logger::info(const std::string &message) { impl->logger->info(message); }
    void Logger::warn(const std::string &message) { impl->logger->warn(message); }
    void Logger::error(const std::string &message) { impl->logger->error(message); }
    void Logger::critical(const std::string &message) { impl->logger->critical(message); }
} // namespace k2