#include "unreal/UnrealProjectDetector.h"
#include "core/Logger.h"
#include <fstream>
#include <QSettings>
#include <QDir>

namespace MyIDE::Unreal {

UnrealProjectDetector& UnrealProjectDetector::instance() {
    static UnrealProjectDetector s_instance;
    return s_instance;
}

UnrealProjectInfo UnrealProjectDetector::detect(const std::filesystem::path& projectPath) {
    UnrealProjectInfo info;
    info.projectPath = projectPath;

    if (!std::filesystem::exists(projectPath)) {
        return info;
    }

    for (const auto& entry : std::filesystem::directory_iterator(projectPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".uproject") {
            info.uprojectFile = entry.path();
            info.projectName = QString::fromStdString(entry.path().stem().string());
            info.isValid = true;
            break;
        }
    }

    if (!info.isValid) return info;

    try {
        std::ifstream file(info.uprojectFile);
        nlohmann::json j;
        file >> j;

        if (j.contains("EngineAssociation")) {
            info.engineAssociation = QString::fromStdString(j["EngineAssociation"].get<std::string>());
        }

        if (j.contains("Modules") && j["Modules"].is_array()) {
            for (const auto& mod : j["Modules"]) {
                UnrealModuleInfo m;
                if (mod.contains("Name")) m.name = QString::fromStdString(mod["Name"]);
                if (mod.contains("Type")) m.type = QString::fromStdString(mod["Type"]);
                if (mod.contains("LoadingPhase")) m.loadingPhase = QString::fromStdString(mod["LoadingPhase"]);
                info.modules.push_back(m);
            }
        }

        info.enginePath = locateEngine(info.engineAssociation);
        MyIDE::Core::Logger::instance().info("UnrealDetector", QString("Detected Unreal Project [%1] Engine: %2 Path: %3")
            .arg(info.projectName)
            .arg(info.engineAssociation)
            .arg(QString::fromStdString(info.enginePath.string())));

    } catch (const std::exception& e) {
        MyIDE::Core::Logger::instance().error("UnrealDetector", QString("Error parsing .uproject file: %1").arg(e.what()));
    }

    return info;
}

std::filesystem::path UnrealProjectDetector::locateEngine(const QString& engineAssociation) {
    // Check Registry for installed Epic Games engine versions
    QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE\\EpicGames\\Unreal Engine", QSettings::NativeFormat);
    QString regPath = registry.value(engineAssociation + "/InstalledDirectory").toString();
    if (!regPath.isEmpty() && std::filesystem::exists(regPath.toStdString())) {
        return regPath.toStdString();
    }

    // Common installation paths search
    std::vector<std::filesystem::path> searchDirs = {
        "C:/Program Files/Epic Games",
        "D:/Epic Games",
        "E:/Epic Games",
        "C:/Program Files (x86)/Epic Games"
    };

    for (const auto& base : searchDirs) {
        if (!std::filesystem::exists(base)) continue;
        for (const auto& entry : std::filesystem::directory_iterator(base)) {
            if (entry.is_directory()) {
                std::string name = entry.path().filename().string();
                if (name.find("UE_") != std::string::npos || name == engineAssociation.toStdString()) {
                    return entry.path();
                }
            }
        }
    }

    return {};
}

bool UnrealProjectDetector::generateCompileCommands(const UnrealProjectInfo& info, const std::filesystem::path& outputPath) {
    if (!info.isValid) return false;

    // Check if compile_commands.json already exists in project or Intermediate folder
    std::filesystem::path existingCommands = info.projectPath / "compile_commands.json";
    if (std::filesystem::exists(existingCommands)) {
        try {
            std::filesystem::copy_file(existingCommands, outputPath, std::filesystem::copy_options::overwrite_existing);
            MyIDE::Core::Logger::instance().info("UnrealDetector", "Copied existing compile_commands.json to .ide workspace");
            return true;
        } catch (...) {}
    }

    // Synthesize compile_commands.json entries for Unreal source modules
    nlohmann::json compileDb = nlohmann::json::array();

    std::vector<std::string> includeFlags;
    if (!info.enginePath.empty()) {
        includeFlags.push_back("-I\"" + (info.enginePath / "Engine/Source/Runtime/Core/Public").string() + "\"");
        includeFlags.push_back("-I\"" + (info.enginePath / "Engine/Source/Runtime/Engine/Classes").string() + "\"");
        includeFlags.push_back("-I\"" + (info.enginePath / "Engine/Source/Runtime/CoreUObject/Public").string() + "\"");
    }

    std::filesystem::path sourceDir = info.projectPath / "Source";
    if (std::filesystem::exists(sourceDir)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(sourceDir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
                std::string command = "clang++ -std=c++20 -DWIN32 -DUBT_COMPILED_PLATFORM_WIN32=1 -DWITH_ENGINE=1 -DUE_BUILD_DEVELOPMENT=1 ";
                for (const auto& inc : includeFlags) command += inc + " ";
                command += "-I\"" + sourceDir.string() + "\" ";
                command += "-c \"" + entry.path().string() + "\"";

                nlohmann::json entryJson = {
                    {"directory", info.projectPath.string()},
                    {"file", entry.path().string()},
                    {"command", command}
                };
                compileDb.push_back(entryJson);
            }
        }
    }

    try {
        std::ofstream outFile(outputPath);
        outFile << compileDb.dump(4);
        MyIDE::Core::Logger::instance().info("UnrealDetector", QString("Generated compile_commands.json for Unreal Engine at %1").arg(QString::fromStdString(outputPath.string())));
        return true;
    } catch (const std::exception& e) {
        MyIDE::Core::Logger::instance().error("UnrealDetector", QString("Failed to write compile_commands.json: %1").arg(e.what()));
        return false;
    }
}

} // namespace MyIDE::Unreal
