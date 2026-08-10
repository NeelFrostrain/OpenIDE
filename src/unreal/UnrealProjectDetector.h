#pragma once

#include <QString>
#include <filesystem>
#include <vector>
#include <nlohmann/json.hpp>

namespace MyIDE::Unreal {

struct UnrealModuleInfo {
    QString name;
    QString type; // "Runtime", "Editor", "Developer"
    QString loadingPhase;
};

struct UnrealProjectInfo {
    bool isValid = false;
    std::filesystem::path projectPath;
    std::filesystem::path uprojectFile;
    QString projectName;
    QString engineAssociation;
    std::filesystem::path enginePath;
    std::vector<UnrealModuleInfo> modules;
    std::vector<QString> plugins;
};

class UnrealProjectDetector {
public:
    static UnrealProjectDetector& instance();

    UnrealProjectInfo detect(const std::filesystem::path& projectPath);
    std::filesystem::path locateEngine(const QString& engineAssociation);
    bool generateCompileCommands(const UnrealProjectInfo& info, const std::filesystem::path& outputPath);

private:
    UnrealProjectDetector() = default;
};

} // namespace MyIDE::Unreal
