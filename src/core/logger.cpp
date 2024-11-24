#include "core/logger.hpp"

#include <spdlog/sinks/ringbuffer_sink.h>

#include <any>

namespace k2::Log {

static LogLevel convert_log_level(const spdlog::level::level_enum& level) {
    switch (level) {
    case spdlog::level::trace: return LogLevel::Trace;
    case spdlog::level::debug: return LogLevel::Debug;
    case spdlog::level::info: return LogLevel::Info;
    case spdlog::level::warn: return LogLevel::Warn;
    case spdlog::level::err: return LogLevel::Err;
    case spdlog::level::critical: return LogLevel::Critical;
    case spdlog::level::off:
    default: return LogLevel::Off;
    }
}

namespace sinks {
    /*
     * Ring buffer sink modification from spdlog.
     * The class in spdlog is final so cannot inherit.
     */
    template <typename Mutex> class ringbuffer_sink final : public spdlog::sinks::base_sink<Mutex> {
    public:
        explicit ringbuffer_sink(size_t n_items)
            : q_ { n_items } { }

        std::vector<std::pair<LogLevel, std::string>> get(size_t lim = 0) {
            [[maybe_unused]] std::lock_guard<Mutex> lock(spdlog::sinks::base_sink<Mutex>::mutex_);
            auto items_available = q_.size();
            auto n_items = lim > 0 ? (std::min)(lim, items_available) : items_available;
            std::vector<std::pair<LogLevel, std::string>> ret;
            ret.reserve(n_items);
            for (size_t i = (items_available - n_items); i < items_available; i++) {
                spdlog::memory_buf_t formatted;
                spdlog::sinks::base_sink<Mutex>::formatter_->format(q_.at(i), formatted);
                ret.emplace_back(convert_log_level(q_.at(i).level), formatted); // TODO
            }
            return ret;
        }

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            q_.push_back(spdlog::details::log_msg_buffer { msg });
        }
        void flush_() override { }

    private:
        spdlog::details::circular_q<spdlog::details::log_msg_buffer> q_;
    };

    using ringbuffer_sink_mt = ringbuffer_sink<std::mutex>;
    using ringbuffer_sink_st = ringbuffer_sink<spdlog::details::null_mutex>;
}

struct Logger::Impl {
    explicit Impl(const std::string& name) {
        auto console_sink = std::make_unique<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink = std::make_unique<spdlog::sinks::basic_file_sink_mt>(std::format("{}.log", name), true);

        console_sink->set_pattern("%^[%T] %n: %v%$");
        file_sink->set_pattern("[%T] [%l] %n: %v");
        std::array<spdlog::sink_ptr, 2> sink_arr = { std::move(console_sink), std::move(file_sink) };

        logger = std::make_unique<spdlog::logger>(name, sink_arr.begin(), sink_arr.end());
        logger->set_level(spdlog::level::trace);
        logger->flush_on(spdlog::level::trace);
    }

    void register_logger(std::any& sink_handle) const {
        logger->sinks().push_back(std::any_cast<spdlog::sink_ptr>(sink_handle));
    }
    std::shared_ptr<spdlog::logger> logger;
};

Logger::Logger(const std::string& name)
    : impl(std::move(std::make_unique<Logger::Impl>(name))) { }

Logger& Logger::add_sink(std::any& sink_handle) {
    impl->register_logger(sink_handle);
    return *this;
}

Logger& Logger::set_level(LogLevel level) {
    switch (level) {
    case LogLevel::Trace: impl->logger->set_level(spdlog::level::trace); break;
    case LogLevel::Debug: impl->logger->set_level(spdlog::level::debug); break;
    case LogLevel::Info: impl->logger->set_level(spdlog::level::info); break;
    case LogLevel::Warn: impl->logger->set_level(spdlog::level::warn); break;
    case LogLevel::Err: impl->logger->set_level(spdlog::level::err); break;
    case LogLevel::Critical: impl->logger->set_level(spdlog::level::critical); break;
    case LogLevel::Off:
    default: impl->logger->set_level(spdlog::level::off);
    }
    return *this;
}

Logger& Logger::trace(const std::string& message) {
    impl->logger->trace(message);
    return *this;
}
Logger& Logger::info(const std::string& message) {
    impl->logger->info(message);
    return *this;
}
Logger& Logger::warn(const std::string& message) {
    impl->logger->warn(message);
    return *this;
}
Logger& Logger::error(const std::string& message) {
    impl->logger->error(message);
    return *this;
}
Logger& Logger::critical(const std::string& message) {
    impl->logger->critical(message);
    return *this;
}

Logger& app() {
    static Logger logger { "App" };
    return logger;
}

Logger& core() {
    static Logger logger { "Core" };
    return logger;
}

template <auto V> struct LoggerInnerType;

template <auto V> using LoggerInnerTypeV = typename LoggerInnerType<V>::type;

template <> struct LoggerInnerType<LoggerSinkType::SingleThreaded> { using type = spdlog::details::null_mutex; };

template <> struct LoggerInnerType<LoggerSinkType::MultiThreaded> { using type = std::mutex; };

template <LoggerSinkType V> using RingBufferSinkT = sinks::ringbuffer_sink<LoggerInnerTypeV<V>>;

template <LoggerSinkType V>
RingBufferSink<V>::RingBufferSink(size_t num_items)
    : impl { std::static_pointer_cast<spdlog::sinks::sink>(std::make_shared<RingBufferSinkT<V>>(num_items)) } { }

template <LoggerSinkType T> std::any& RingBufferSink<T>::get_sink() { return impl; }

template <LoggerSinkType V> std::vector<std::pair<LogLevel, std::string>> RingBufferSink<V>::get(size_t num_items) {
    return std::reinterpret_pointer_cast<RingBufferSinkT<V>>(std::any_cast<spdlog::sink_ptr>(impl))->get(num_items);
}

template <LoggerSinkType V> void RingBufferSink<V>::set_pattern(const std::string& pattern) {
    return std::reinterpret_pointer_cast<RingBufferSinkT<V>>(std::any_cast<spdlog::sink_ptr>(impl))
        ->set_pattern(pattern);
}

template class RingBufferSink<LoggerSinkType::SingleThreaded>;
template class RingBufferSink<LoggerSinkType::MultiThreaded>;
}
