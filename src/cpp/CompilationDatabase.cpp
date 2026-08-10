#include "cpp/CompilationDatabase.h"
#include "unreal/UnrealProjectDetector.h"
#include "core/Logger.h"
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>

namespace MyIDE::Cpp {

CompilationDatabase& CompilationDatabase::instance() {
    static CompilationDatabase s_instance;
    return s_instance;
}

bool CompilationDatabase::ensureCompilationDatabase(const Project::ProjectPaths& paths) {
    std::filesystem::path lspDir = paths.lsp();
    std::filesystem::path targetDb = paths.compileCommandsFile();

    std::error_code ec;
    std::filesystem::create_directories(lspDir, ec);

    MyIDE::Core::Logger::instance().info("LSP", QString("[LSP] Generating compilation database at %1")
        .arg(QString::fromStdString(targetDb.string())));

    std::filesystem::path userRootDb = paths.root / "compile_commands.json";
    if (std::filesystem::exists(userRootDb)) {
        MyIDE::Core::Logger::instance().info("LSP", QString("[Project] User-owned compile_commands.json found at project root. Copying to %1")
            .arg(QString::fromStdString(targetDb.string())));
        try {
            std::filesystem::copy_file(userRootDb, targetDb, std::filesystem::copy_options::overwrite_existing);
        } catch (...) {}
    }

    // Check if project is Unreal Engine
    auto unrealInfo = Unreal::UnrealProjectDetector::instance().detect(paths.root);
    if (unrealInfo.isValid) {
        bool res = Unreal::UnrealProjectDetector::instance().generateCompileCommands(unrealInfo, targetDb);
        if (res && std::filesystem::exists(targetDb) && std::filesystem::file_size(targetDb) > 0) {
            MyIDE::Core::Logger::instance().info("LSP", QString("[LSP] compile_commands.json verified size=%1 bytes")
                .arg(std::filesystem::file_size(targetDb)));
            return true;
        }
    }

    // Default C++ compilation database generator for generic C++ workspace
    nlohmann::json compileDb = nlohmann::json::array();
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(paths.root)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".cpp" || ext == ".c" || ext == ".cc") {
                    nlohmann::json item = {
                        {"directory", paths.root.string()},
                        {"file", entry.path().string()},
                        {"command", "clang++ -std=c++20 -I\"" + paths.root.string() + "\" -c \"" + entry.path().string() + "\""}
                    };
                    compileDb.push_back(item);
                }
            }
        }

        std::ofstream outFile(targetDb);
        outFile << compileDb.dump(4);
        MyIDE::Core::Logger::instance().info("LSP", QString("[LSP] Generated default compile_commands.json with %1 entries").arg(compileDb.size()));
        return true;
    } catch (const std::exception& e) {
        MyIDE::Core::Logger::instance().error("LSP", QString("[LSP] Error generating default compile_commands.json: %1").arg(e.what()));
        return false;
    }
}

std::vector<std::string> CompilationDatabase::extractIncludePaths(const std::filesystem::path& compileDbPath) {
    std::vector<std::string> paths;
    if (!std::filesystem::exists(compileDbPath)) return paths;

    try {
        std::ifstream file(compileDbPath);
        nlohmann::json j;
        file >> j;

        for (const auto& entry : j) {
            std::string cmd;
            if (entry.contains("command")) {
                cmd = entry["command"].get<std::string>();
            }

            size_t pos = 0;
            while ((pos = cmd.find("-I", pos)) != std::string::npos) {
                pos += 2;
                while (pos < cmd.length() && (cmd[pos] == ' ' || cmd[pos] == '"')) pos++;
                size_t endPos = pos;
                while (endPos < cmd.length() && cmd[endPos] != '"' && cmd[endPos] != ' ') endPos++;
                if (endPos > pos) {
                    std::string incPath = cmd.substr(pos, endPos - pos);
                    if (std::find(paths.begin(), paths.end(), incPath) == paths.end()) {
                        paths.push_back(incPath);
                    }
                }
                pos = endPos;
            }
        }
    } catch (...) {}

    return paths;
}

std::vector<std::string> CompilationDatabase::extractDefines(const std::filesystem::path& compileDbPath) {
    std::vector<std::string> defs;
    if (!std::filesystem::exists(compileDbPath)) return defs;

    try {
        std::ifstream file(compileDbPath);
        nlohmann::json j;
        file >> j;

        for (const auto& entry : j) {
            std::string cmd;
            if (entry.contains("command")) {
                cmd = entry["command"].get<std::string>();
            }

            size_t pos = 0;
            while ((pos = cmd.find("-D", pos)) != std::string::npos) {
                pos += 2;
                while (pos < cmd.length() && (cmd[pos] == ' ' || cmd[pos] == '"')) pos++;
                size_t endPos = pos;
                while (endPos < cmd.length() && cmd[endPos] != '"' && cmd[endPos] != ' ') endPos++;
                if (endPos > pos) {
                    std::string defStr = cmd.substr(pos, endPos - pos);
                    if (std::find(defs.begin(), defs.end(), defStr) == defs.end()) {
                        defs.push_back(defStr);
                    }
                }
                pos = endPos;
            }
        }
    } catch (...) {}

    return defs;
}

std::string CompilationDatabase::getCommandForFile(const std::filesystem::path& compileDbPath, const std::filesystem::path& filePath) {
    if (!std::filesystem::exists(compileDbPath)) return {};

    try {
        std::ifstream file(compileDbPath);
        nlohmann::json j;
        file >> j;

        std::string targetStr = filePath.string();
        for (const auto& entry : j) {
            if (entry.contains("file") && entry["file"].get<std::string>() == targetStr) {
                return entry.value("command", "");
            }
        }
    } catch (...) {}

    return {};
}

} // namespace MyIDE::Cpp
