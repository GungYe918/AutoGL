#include <AutoGL/Log.hpp>
#include <iostream>
#include <mutex>

namespace AutoGL::Log::detail {
    std::mutex& logMutex() {
        static std::mutex m;
        return m;
    }

    LogSink& globalSink() {
        static LogSink sink = nullptr;
        return sink;
    }

    constexpr const char* levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Trace: return "TRACE";
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info:  return "INFO";
            case LogLevel::Warn:  return "WARN";
            case LogLevel::Error: return "ERROR";
            case LogLevel::Fatal: return "FATAL";
        }

        return "UNKNOWN";
    }

} // namespace AutoGL::Log::detail

namespace AutoGL::Log {

    void setLogSink(LogSink sink) noexcept {
        std::lock_guard<std::mutex> lock(detail::logMutex());
        detail::globalSink() = sink;
    }

    void resetLogSink() noexcept {
        std::lock_guard<std::mutex> lock(detail::logMutex());
        detail::globalSink() = nullptr;
    }

    void Log(
        LogLevel level,
        const char* category,
        const std::string& message
    ) {
        std::lock_guard<std::mutex> lock(detail::logMutex());

        if (auto sink = detail::globalSink()) {
            sink(level, category, message);
            return;
        }

        std::cerr << "[AutoGL][" << detail::levelToString(level)
                  << "][" << (category ? category : "core") << "] "
                  << message << '\n';
    }

} // namespace AutoGL