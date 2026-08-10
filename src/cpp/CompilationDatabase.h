#pragma once

#include "project/ProjectManager.h"
#include <QString>
#include <filesystem>
#include <vector>

namespace MyIDE::Cpp {

class CompilationDatabase {
public:
    static CompilationDatabase& instance();

    bool ensureCompilationDatabase(const Project::ProjectPaths& paths);

private:
    CompilationDatabase() = default;
};

} // namespace MyIDE::Cpp
