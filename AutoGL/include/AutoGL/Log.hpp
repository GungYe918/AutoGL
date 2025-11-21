#pragma once
#include <string>

namespace AutoGL::Log {

    enum class LogLevel : uint8_t {
        Trace = 0,
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };

    using LogSink = void(*)(
        LogLevel level,
        const char* category,
        const std::string& message
    );

    void setLogSink(LogSink sink) noexcept;
    void resetLogSink() noexcept;

    void Log(
        LogLevel level,
        const char* category,
        const std::string& message
    );

} // namespace AutoGL

#define AUTOGL_LOG_TRACE(cat, msg) ::AutoGL::Log::Log(::AutoGL::Log::LogLevel::Trace, cat, msg)
#define AUTOGL_LOG_DEBUG(cat, msg) ::AutoGL::Log::Log(::AutoGL::Log::LogLevel::Debug, cat, msg)
#define AUTOGL_LOG_INFO(cat,  msg) ::AutoGL::Log::Log(::AutoGL::Log::LogLevel::Info,  cat, msg)
#define AUTOGL_LOG_WARN(cat,  msg) ::AutoGL::Log::Log(::AutoGL::Log::LogLevel::Warn,  cat, msg)
#define AUTOGL_LOG_ERROR(cat, msg) ::AutoGL::Log::Log(::AutoGL::Log::LogLevel::Error, cat, msg)
#define AUTOGL_LOG_FATAL(cat, msg) ::AutoGL::Log::Log(::AutoGL::Log::LogLevel::Fatal, cat, msg)