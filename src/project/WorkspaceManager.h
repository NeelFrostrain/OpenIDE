#pragma once

#include <QString>
#include <filesystem>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace MyIDE::Project {

struct CursorPos {
    int line = 1;
    int column = 1;
};

struct WorkspaceState {
    int version = 1;
    std::string activeFile;
    std::vector<std::string> openFiles;
    std::map<std::string, CursorPos> cursorPositions;
    std::string buildConfiguration = "Development Editor";
    std::string platform = "Win64";
    std::string target = "MyGameEditor";
};

class WorkspaceManager {
public:
    static WorkspaceManager& instance();

    void initializeWorkspace(const std::filesystem::path& ideRoot);
    void saveWorkspace(const WorkspaceState& state);
    WorkspaceState loadWorkspace();

    void saveProjectState(const std::string& buildConfig, const std::string& platform, const std::string& target);
    void saveSymbolIndex(const std::vector<std::string>& files, int totalSymbols);
    void saveCache(const std::string& key, const std::string& value);
    void saveLspConfig(const std::string& clangdPath, const std::string& compileDbDir);

private:
    WorkspaceManager() = default;
    std::filesystem::path m_ideRoot;
};

} // namespace MyIDE::Project
