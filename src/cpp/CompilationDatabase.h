#pragma once

#include "project/ProjectManager.h"
#include <QString>
#include <filesystem>
#include <vector>
#include <string>

namespace OpenIDE::Cpp {

class CompilationDatabase {
public:
    static CompilationDatabase& instance();

    bool ensureCompilationDatabase(const Project::ProjectPaths& paths);

    std::vector<std::string> extractIncludePaths(const std::filesystem::path& compileDbPath);
    std::vector<std::string> extractDefines(const std::filesystem::path& compileDbPath);
    std::string getCommandForFile(const std::filesystem::path& compileDbPath, const std::filesystem::path& filePath);

private:
    CompilationDatabase() = default;
};

} // namespace OpenIDE::Cpp
