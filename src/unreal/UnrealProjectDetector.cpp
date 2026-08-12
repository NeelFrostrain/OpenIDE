#include "unreal/UnrealProjectDetector.h"
#include "language/IncludeIndex.h"
#include "core/Logger.h"
#include <QDir>
#include <QSettings>
#include <fstream>

namespace OpenIDE::Unreal {

UnrealProjectDetector &UnrealProjectDetector::instance() {
  static UnrealProjectDetector s_instance;
  return s_instance;
}

UnrealProjectInfo
UnrealProjectDetector::detect(const std::filesystem::path &projectPath) {
  UnrealProjectInfo info;
  info.projectPath = projectPath;

  if (!std::filesystem::exists(projectPath)) {
    return info;
  }

  for (const auto &entry : std::filesystem::directory_iterator(projectPath)) {
    if (entry.is_regular_file() && entry.path().extension() == ".uproject") {
      info.uprojectFile = entry.path();
      info.projectName = QString::fromStdString(entry.path().stem().string());
      info.isValid = true;
      break;
    }
  }

  if (!info.isValid)
    return info;

  try {
    std::ifstream file(info.uprojectFile);
    nlohmann::json j;
    file >> j;

    if (j.contains("EngineAssociation")) {
      info.engineAssociation =
          QString::fromStdString(j["EngineAssociation"].get<std::string>());
    }

    if (j.contains("Modules") && j["Modules"].is_array()) {
      for (const auto &mod : j["Modules"]) {
        UnrealModuleInfo m;
        if (mod.contains("Name"))
          m.name = QString::fromStdString(mod["Name"]);
        if (mod.contains("Type"))
          m.type = QString::fromStdString(mod["Type"]);
        if (mod.contains("LoadingPhase"))
          m.loadingPhase = QString::fromStdString(mod["LoadingPhase"]);
        info.modules.push_back(m);
      }
    }

    info.enginePath = locateEngine(info.engineAssociation);
    OpenIDE::Core::Logger::instance().info(
        "UnrealDetector",
        QString("Detected Unreal Project [%1] Engine: %2 Path: %3")
            .arg(info.projectName)
            .arg(info.engineAssociation)
            .arg(QString::fromStdString(info.enginePath.string())));

  } catch (const std::exception &e) {
    OpenIDE::Core::Logger::instance().error(
        "UnrealDetector",
        QString("Error parsing .uproject file: %1").arg(e.what()));
  }

  return info;
}

std::filesystem::path
UnrealProjectDetector::locateEngine(const QString &engineAssociation) {
  // Check Registry for installed Epic Games engine versions
  QSettings registry("HKEY_LOCAL_MACHINE\\SOFTWARE\\EpicGames\\Unreal Engine",
                     QSettings::NativeFormat);
  QString regPath =
      registry.value(engineAssociation + "/InstalledDirectory").toString();
  if (!regPath.isEmpty() && std::filesystem::exists(regPath.toStdString())) {
    return regPath.toStdString();
  }

  // Common installation paths search
  std::vector<std::filesystem::path> searchDirs = {
      "C:/Program Files/Epic Games", "D:/Epic Games", "E:/Epic Games",
      "C:/Program Files (x86)/Epic Games"};

  for (const auto &base : searchDirs) {
    if (!std::filesystem::exists(base))
      continue;
    for (const auto &entry : std::filesystem::directory_iterator(base)) {
      if (entry.is_directory()) {
        std::string name = entry.path().filename().string();
        if (name.find("UE_") != std::string::npos ||
            name == engineAssociation.toStdString()) {
          return entry.path();
        }
      }
    }
  }

  return {};
}

bool UnrealProjectDetector::generateCompileCommands(
    const UnrealProjectInfo &info, const std::filesystem::path &outputPath) {
  if (!info.isValid)
    return false;

  // Check if compile_commands.json already exists in project or Intermediate
  std::filesystem::path existingCommands = info.projectPath / "compile_commands.json";
  std::filesystem::path intermediateCommands = info.projectPath / "Intermediate" / "Build" / "Win64" / "compile_commands.json";

  std::filesystem::path sourceCommands = std::filesystem::exists(existingCommands) ? existingCommands
                                        : (std::filesystem::exists(intermediateCommands) ? intermediateCommands : std::filesystem::path());

  if (!sourceCommands.empty()) {
    try {
      std::filesystem::create_directories(outputPath.parent_path());
      std::filesystem::copy_file(
          sourceCommands, outputPath,
          std::filesystem::copy_options::overwrite_existing);
      OpenIDE::Core::Logger::instance().info(
          "UnrealDetector",
          QString("Copied existing compile_commands.json from %1 to %2")
              .arg(QString::fromStdString(sourceCommands.string()))
              .arg(QString::fromStdString(outputPath.string())));
      return true;
    } catch (...) {
    }
  }

  // Synthesize compile_commands.json entries for Unreal source modules
  nlohmann::json compileDb = nlohmann::json::array();

  std::vector<std::string> includeDirs;
  if (!info.enginePath.empty()) {
    std::vector<std::string> engineSubDirs = {
        "Engine/Source/Runtime/Core/Public",
        "Engine/Source/Runtime/CoreUObject/Public",
        "Engine/Source/Runtime/Engine/Classes",
        "Engine/Source/Runtime/Engine/Public",
        "Engine/Source/Runtime/InputCore/Classes",
        "Engine/Source/Runtime/InputCore/Public",
        "Engine/Source/Runtime/Slate/Public",
        "Engine/Source/Runtime/SlateCore/Public",
        "Engine/Source/Runtime/UMG/Public",
        "Engine/Source/Runtime/Projects/Public",
        "Engine/Source/Runtime/RenderCore/Public",
        "Engine/Source/Runtime/RHI/Public",
        "Engine/Source/Runtime/TraceLog/Public",
        "Engine/Source/Developer/TargetPlatform/Public",
        "Engine/Intermediate/Build/Win64/UnrealEditor/Inc/CoreUObject",
        "Engine/Intermediate/Build/Win64/UnrealEditor/Inc/Engine"
    };

    for (const auto& sub : engineSubDirs) {
      std::filesystem::path fullP = info.enginePath / sub;
      if (std::filesystem::exists(fullP)) {
        includeDirs.push_back(fullP.string());
      }
    }
  }

  std::filesystem::path sourceDir = info.projectPath / "Source";
  if (std::filesystem::exists(sourceDir)) {
    includeDirs.push_back(sourceDir.string());
    if (!info.projectName.isEmpty()) {
      includeDirs.push_back((sourceDir / info.projectName.toStdString()).string());
    }
  }

  std::string command = "clang++ -std=c++20 -DWIN32=1 -D_WIN64=1 -DPLATFORM_WINDOWS=1 -DUBT_COMPILED_PLATFORM_WIN64=1 "
                        "-DWITH_ENGINE=1 -DWITH_EDITOR=1 -DWITH_COREUOBJECT=1 -DUE_BUILD_DEVELOPMENT=1 -DUE_EDITOR=1 ";

  for (const auto &inc : includeDirs) {
    command += "-I\"" + inc + "\" ";
  }

  if (std::filesystem::exists(sourceDir)) {
    for (const auto &entry : std::filesystem::recursive_directory_iterator(sourceDir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
        std::string entryCmd = command + "-c \"" + entry.path().string() + "\"";
        nlohmann::json entryJson = {
            {"directory", info.projectPath.string()},
            {"file", entry.path().string()},
            {"command", entryCmd}
        };
        compileDb.push_back(entryJson);
      }
    }
  }

  try {
    std::filesystem::create_directories(outputPath.parent_path());
    std::ofstream outFile(outputPath);
    outFile << compileDb.dump(4);
    OpenIDE::Core::Logger::instance().info(
        "UnrealDetector",
        QString("Generated compilation database for Unreal Engine at %1 with %2 files")
            .arg(QString::fromStdString(outputPath.string()))
            .arg(compileDb.size()));
    return true;
  } catch (const std::exception &e) {
    OpenIDE::Core::Logger::instance().error(
        "UnrealDetector",
        QString("Failed to write compile_commands.json: %1").arg(e.what()));
    return false;
  }
}

} // namespace OpenIDE::Unreal
