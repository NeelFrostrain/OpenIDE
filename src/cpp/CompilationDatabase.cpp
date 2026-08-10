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

        if (std::filesystem::exists(targetDb) && std::filesystem::file_size(targetDb) > 0) {
            MyIDE::Core::Logger::instance().info("LSP", QString("[LSP] compile_commands.json verified size=%1 bytes entries=%2")
                .arg(std::filesystem::file_size(targetDb))
                .arg(compileDb.size()));
            MyIDE::Core::Logger::instance().info("LSP", "[LSP] Compilation database ready");
            return true;
        } else {
            MyIDE::Core::Logger::instance().error("LSP", QString("[LSP] Failed to verify generated compilation database at %1").arg(QString::fromStdString(targetDb.string())));
            return false;
        }
    } catch (const std::exception& e) {
        MyIDE::Core::Logger::instance().error("LSP", QString("[LSP] Failed to generate compilation database: %1").arg(e.what()));
        return false;
    }
}

} // namespace MyIDE::Cpp
