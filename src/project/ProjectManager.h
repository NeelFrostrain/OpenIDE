#pragma once

#include <QObject>
#include <filesystem>
#include <vector>
#include <QString>

namespace MyIDE::Project {

enum class ProjectType {
    GenericCpp,
    CMake,
    UnrealEngine
};

class ProjectPaths {
public:
    std::filesystem::path root;

    std::filesystem::path ide() const { return root / ".ide"; }
    std::filesystem::path cache() const { return ide() / "cache"; }
    std::filesystem::path index() const { return ide() / "index"; }
    std::filesystem::path lsp() const { return ide() / "lsp"; }
    std::filesystem::path logs() const { return ide() / "logs"; }
    std::filesystem::path state() const { return ide() / "state"; }
    std::filesystem::path workspace() const { return ide() / "workspace"; }
    std::filesystem::path temp() const { return ide() / "tmp"; }
    std::filesystem::path compileCommandsFile() const { return lsp() / "compile_commands.json"; }
};

class ProjectManager : public QObject {
    Q_OBJECT

public:
    static ProjectManager& instance();

    bool openProject(const std::filesystem::path& projectPath);
    void closeProject();

    bool isProjectOpen() const { return m_isOpen; }
    std::filesystem::path projectPath() const { return m_paths.root; }
    ProjectPaths paths() const { return m_paths; }
    QString projectName() const { return m_projectName; }
    ProjectType projectType() const { return m_projectType; }

    std::vector<std::filesystem::path> sourceFiles() const;

signals:
    void projectOpened(const QString& name, ProjectType type);
    void projectClosed();
    void fileStructureChanged();

private:
    ProjectManager() = default;
    
    void initializeIdeDirectory();
    void ensureGitIgnoreEntries();
    void detectProjectType();
    void scanProjectFiles();

    bool m_isOpen = false;
    ProjectPaths m_paths;
    QString m_projectName;
    ProjectType m_projectType = ProjectType::GenericCpp;
    std::vector<std::filesystem::path> m_files;
};

} // namespace MyIDE::Project
