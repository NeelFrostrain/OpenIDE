#pragma once

#include <QString>
#include <filesystem>
#include <vector>

namespace MyIDE::Cpp {

class CompilationDatabase {
public:
    static CompilationDatabase& instance();

    bool ensureCompilationDatabase(const std::filesystem::path& projectPath, const std::filesystem::path& idePath);

private:
    CompilationDatabase() = default;
};

} // namespace MyIDE::Cpp
