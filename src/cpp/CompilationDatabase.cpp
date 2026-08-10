#include "cpp/CompilationDatabase.h"
#include "unreal/UnrealProjectDetector.h"
#include "core/Logger.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace MyIDE::Cpp {

CompilationDatabase& CompilationDatabase::instance() {
    static CompilationDatabase s_instance;
    return s_instance;
}

bool CompilationDatabase::ensureCompilationDatabase(const std::filesystem::path& projectPath, const std::filesystem::path& idePath) {
    std::filesystem::path rootDb = projectPath / "compile_commands.json";
    std::filesystem::path ideDb = idePath / "compile_commands.json";

    if (std::filesystem::exists(rootDb)) {
        MyIDE::Core::Logger::instance().info("CompilationDatabase", "Found root compile_commands.json");
        return true;
    }

    // Check if project is Unreal Engine
    auto unrealInfo = Unreal::UnrealProjectDetector::instance().detect(projectPath);
    if (unrealInfo.isValid) {
        return Unreal::UnrealProjectDetector::instance().generateCompileCommands(unrealInfo, rootDb);
    }

    // Default C++ compilation database generator for generic C++ workspace
    nlohmann::json compileDb = nlohmann::json::array();
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(projectPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".cpp" || ext == ".c" || ext == ".cc") {
                    nlohmann::json item = {
                        {"directory", projectPath.string()},
                        {"file", entry.path().string()},
                        {"command", "clang++ -std=c++20 -I\"" + projectPath.string() + "\" -c \"" + entry.path().string() + "\""}
                    };
                    compileDb.push_back(item);
                }
            }
        }

        std::ofstream outFile(rootDb);
        outFile << compileDb.dump(4);
        MyIDE::Core::Logger::instance().info("CompilationDatabase", "Generated default C++ compile_commands.json");
        return true;
    } catch (const std::exception& e) {
        MyIDE::Core::Logger::instance().error("CompilationDatabase", QString("Failed to create compile_commands.json: %1").arg(e.what()));
        return false;
    }
}

} // namespace MyIDE::Cpp
