#include "core/Config.h"
#include "core/Logger.h"
#include <fstream>
#include <QStandardPaths>
#include <QDir>
#include <QProcessEnvironment>
#include <QFileInfo>

namespace MyIDE::Core {

Config& Config::instance() {
    static Config s_instance;
    return s_instance;
}

bool Config::load(const std::filesystem::path& configPath) {
    m_configPath = configPath;
    if (m_configPath.empty()) {
        QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(appData);
        m_configPath = (appData + "/settings.json").toStdString();
    }

    if (std::filesystem::exists(m_configPath)) {
        try {
            std::ifstream file(m_configPath);
            nlohmann::json j;
            file >> j;

            if (j.contains("clangdExecutable")) m_clangdExecutable = QString::fromStdString(j["clangdExecutable"]);
            if (j.contains("cmakeExecutable"))  m_cmakeExecutable  = QString::fromStdString(j["cmakeExecutable"]);
            if (j.contains("ninjaExecutable"))  m_ninjaExecutable  = QString::fromStdString(j["ninjaExecutable"]);
            if (j.contains("tabSize"))          m_tabSize          = j["tabSize"];
            if (j.contains("insertSpaces"))     m_insertSpaces     = j["insertSpaces"];
            if (j.contains("theme"))            m_theme            = QString::fromStdString(j["theme"]);
        } catch (const std::exception& e) {
            LOG_ERROR("Config", QString("Failed to parse config file: %1").arg(e.what()));
        }
    }

    // Auto-discover paths if not configured
    if (m_clangdExecutable.isEmpty()) {
        m_clangdExecutable = "clangd";
    }
    if (m_cmakeExecutable.isEmpty()) {
        m_cmakeExecutable = "cmake";
    }
    if (m_ninjaExecutable.isEmpty()) {
        m_ninjaExecutable = "ninja";
    }

    return true;
}

bool Config::save() const {
    if (m_configPath.empty()) return false;
    try {
        nlohmann::json j;
        j["clangdExecutable"] = m_clangdExecutable.toStdString();
        j["cmakeExecutable"]  = m_cmakeExecutable.toStdString();
        j["ninjaExecutable"]  = m_ninjaExecutable.toStdString();
        j["tabSize"]          = m_tabSize;
        j["insertSpaces"]     = m_insertSpaces;
        j["theme"]            = m_theme.toStdString();

        std::ofstream file(m_configPath);
        file << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Config", QString("Failed to save config file: %1").arg(e.what()));
        return false;
    }
}

QString Config::clangdExecutable() const { return m_clangdExecutable; }
void Config::setClangdExecutable(const QString& path) { m_clangdExecutable = path; }

QString Config::cmakeExecutable() const { return m_cmakeExecutable; }
void Config::setCmakeExecutable(const QString& path) { m_cmakeExecutable = path; }

QString Config::ninjaExecutable() const { return m_ninjaExecutable; }
void Config::setNinjaExecutable(const QString& path) { m_ninjaExecutable = path; }

} // namespace MyIDE::Core
