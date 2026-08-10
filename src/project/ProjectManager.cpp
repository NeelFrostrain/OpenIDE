#include "project/ProjectManager.h"
#include "cpp/CompilationDatabase.h"
#include "core/Logger.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace MyIDE::Project {

ProjectManager& ProjectManager::instance() {
    static ProjectManager s_instance;
    return s_instance;
}

bool ProjectManager::openProject(const std::filesystem::path& projectPath) {
    if (!std::filesystem::exists(projectPath)) {
        MyIDE::Core::Logger::instance().error("ProjectManager", "Project path does not exist");
        return false;
    }

    m_paths.root = std::filesystem::canonical(projectPath);
    m_projectName = QString::fromStdString(m_paths.root.filename().string());
    m_isOpen = true;

    initializeIdeDirectory();
    ensureGitIgnoreEntries();
    detectProjectType();
    
    // Ensure compilation database for clangd include paths & defines
    Cpp::CompilationDatabase::instance().ensureCompilationDatabase(m_paths.root, m_paths.ide());

    scanProjectFiles();

    emit projectOpened(m_projectName, m_projectType);
    MyIDE::Core::Logger::instance().info("ProjectManager", QString("Opened project [%1] at %2").arg(m_projectName).arg(QString::fromStdString(m_paths.root.string())));
    return true;
}

void ProjectManager::closeProject() {
    if (m_isOpen) {
        m_isOpen = false;
        m_files.clear();
        emit projectClosed();
        MyIDE::Core::Logger::instance().info("ProjectManager", "Project closed");
    }
}

void ProjectManager::initializeIdeDirectory() {
    try {
        std::filesystem::create_directories(m_paths.ide());
        std::filesystem::create_directories(m_paths.cache());
        std::filesystem::create_directories(m_paths.index());
        std::filesystem::create_directories(m_paths.lsp());
        std::filesystem::create_directories(m_paths.logs());
        std::filesystem::create_directories(m_paths.state());
        std::filesystem::create_directories(m_paths.workspace());
        std::filesystem::create_directories(m_paths.temp());

        // Write or update .ide/version.json
        nlohmann::json versionJson = {
            {"formatVersion", 1},
            {"myideVersion", "0.1.0"}
        };
        std::ofstream verFile(m_paths.ide() / "version.json");
        verFile << versionJson.dump(4);
        
        MyIDE::Core::Logger::instance().info("ProjectManager", "Initialized project-local .ide/ workspace directory");
    } catch (const std::exception& e) {
        MyIDE::Core::Logger::instance().error("ProjectManager", QString("Failed to initialize .ide directory: %1").arg(e.what()));
    }
}

void ProjectManager::ensureGitIgnoreEntries() {
    std::filesystem::path gitIgnorePath = m_paths.root / ".gitignore";
    bool ideIgnored = false;

    if (std::filesystem::exists(gitIgnorePath)) {
        std::ifstream inFile(gitIgnorePath);
        std::string line;
        while (std::getline(inFile, line)) {
            if (line.find(".ide/") != std::string::npos || line.find(".ide") != std::string::npos) {
                ideIgnored = true;
                break;
            }
        }
    }

    if (!ideIgnored) {
        std::ofstream outFile(gitIgnorePath, std::ios::app);
        outFile << "\n# MyIDE workspace directory\n.ide/\n";
        MyIDE::Core::Logger::instance().info("ProjectManager", "Added .ide/ entry to project .gitignore");
    }
}

void ProjectManager::detectProjectType() {
    m_projectType = ProjectType::GenericCpp;

    for (const auto& entry : std::filesystem::directory_iterator(m_paths.root)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            std::string name = entry.path().filename().string();

            if (ext == ".uproject") {
                m_projectType = ProjectType::UnrealEngine;
                MyIDE::Core::Logger::instance().info("ProjectManager", "Detected Unreal Engine Project");
                return;
            } else if (name == "CMakeLists.txt") {
                m_projectType = ProjectType::CMake;
                MyIDE::Core::Logger::instance().info("ProjectManager", "Detected CMake Project");
            }
        }
    }
}

void ProjectManager::scanProjectFiles() {
    m_files.clear();
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(m_paths.root)) {
            // Ignore hidden directories like .git and .ide
            if (entry.is_directory()) {
                std::string dirName = entry.path().filename().string();
                if (dirName == ".git" || dirName == ".ide" || dirName == "Intermediate" || dirName == "Binaries") {
                    continue;
                }
            }
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".cpp" || ext == ".h" || ext == ".hpp" || ext == ".c" || ext == ".cs" || ext == ".json") {
                    m_files.push_back(entry.path());
                }
            }
        }
    } catch (const std::exception& e) {
        MyIDE::Core::Logger::instance().error("ProjectManager", QString("Error scanning project files: %1").arg(e.what()));
    }
}

std::vector<std::filesystem::path> ProjectManager::sourceFiles() const {
    return m_files;
}

} // namespace MyIDE::Project
