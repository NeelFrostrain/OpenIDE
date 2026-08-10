#include "core/Logger.h"
#include <QDateTime>
#include <QDebug>
#include <iostream>

namespace MyIDE::Core {

Logger& Logger::instance() {
    static Logger s_instance;
    return s_instance;
}

Logger::~Logger() {
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

void Logger::init(const QString& logFilePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!logFilePath.isEmpty()) {
        m_logFile.open(logFilePath.toStdString(), std::ios::out | std::ios::app);
    }
    m_initialized = true;
}

void Logger::log(LogLevel level, const QString& category, const QString& message) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    const char* levelStr = "INFO";
    switch (level) {
        case LogLevel::Trace:   levelStr = "TRACE"; break;
        case LogLevel::Debug:   levelStr = "DEBUG"; break;
        case LogLevel::Info:    levelStr = "INFO "; break;
        case LogLevel::Warning: levelStr = "WARN "; break;
        case LogLevel::Error:   levelStr = "ERROR"; break;
        case LogLevel::Fatal:   levelStr = "FATAL"; break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString formatted = QString("[%1] [%2] [%3] %4")
                            .arg(timestamp)
                            .arg(levelStr)
                            .arg(category)
                            .arg(message);

    std::cout << formatted.toStdString() << std::endl;

    if (m_logFile.is_open()) {
        m_logFile << formatted.toStdString() << std::endl;
    }

    emit logEmitted(level, category, message, formatted);
}

} // namespace MyIDE::Core
