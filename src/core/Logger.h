#pragma once

#include <QString>
#include <QObject>
#include <mutex>
#include <fstream>
#include <memory>

namespace OpenIDE::Core {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

class Logger : public QObject {
    Q_OBJECT

public:
    static Logger& instance();

    void init(const QString& logFilePath = QString());
    void log(LogLevel level, const QString& category, const QString& message);

    void trace(const QString& category, const QString& msg) { log(LogLevel::Trace, category, msg); }
    void debug(const QString& category, const QString& msg) { log(LogLevel::Debug, category, msg); }
    void info(const QString& category, const QString& msg) { log(LogLevel::Info, category, msg); }
    void warn(const QString& category, const QString& msg) { log(LogLevel::Warning, category, msg); }
    void error(const QString& category, const QString& msg) { log(LogLevel::Error, category, msg); }
    void fatal(const QString& category, const QString& msg) { log(LogLevel::Fatal, category, msg); }

signals:
    void logEmitted(LogLevel level, const QString& category, const QString& message, const QString& formattedMessage);

private:
    Logger() = default;
    ~Logger();

    std::mutex m_mutex;
    std::ofstream m_logFile;
    bool m_initialized = false;
};

} // namespace OpenIDE::Core

#define LOG_TRACE(cat, msg) OpenIDE::Core::Logger::instance().trace(cat, msg)
#define LOG_DEBUG(cat, msg) OpenIDE::Core::Logger::instance().debug(cat, msg)
#define LOG_INFO(cat, msg)  OpenIDE::Core::Logger::instance().info(cat, msg)
#define LOG_WARN(cat, msg)  OpenIDE::Core::Logger::instance().warn(cat, msg)
#define LOG_ERROR(cat, msg) OpenIDE::Core::Logger::instance().error(cat, msg)
#define LOG_FATAL(cat, msg) OpenIDE::Core::Logger::instance().fatal(cat, msg)
