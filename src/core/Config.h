#pragma once

#include <QString>
#include <QVariantMap>
#include <nlohmann/json.hpp>
#include <filesystem>

namespace OpenIDE::Core {

class Config {
public:
    static Config& instance();

    bool load(const std::filesystem::path& configPath);
    bool save() const;

    QString clangdExecutable() const;
    void setClangdExecutable(const QString& path);

    QString cmakeExecutable() const;
    void setCmakeExecutable(const QString& path);

    QString ninjaExecutable() const;
    void setNinjaExecutable(const QString& path);

    int tabSize() const { return m_tabSize; }
    void setTabSize(int size) { m_tabSize = size; }

    bool insertSpaces() const { return m_insertSpaces; }
    void setInsertSpaces(bool spaces) { m_insertSpaces = spaces; }

    QString theme() const { return m_theme; }
    void setTheme(const QString& theme) { m_theme = theme; }

private:
    Config() = default;

    std::filesystem::path m_configPath;
    QString m_clangdExecutable;
    QString m_cmakeExecutable;
    QString m_ninjaExecutable;
    int m_tabSize = 4;
    bool m_insertSpaces = true;
    QString m_theme = "Dark";
};

} // namespace OpenIDE::Core
