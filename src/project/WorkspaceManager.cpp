#include "project/WorkspaceManager.h"
#include "core/Logger.h"
#include <QDateTime>
#include <fstream>
#include <QFileInfo>

namespace OpenIDE::Project {

WorkspaceManager& WorkspaceManager::instance() {
    static WorkspaceManager s_instance;
    return s_instance;
}

void WorkspaceManager::initializeWorkspace(const std::filesystem::path& ideRoot) {
    m_ideRoot = ideRoot;
    OpenIDE::Core::Logger::instance().info("WorkspaceManager", QString("Workspace storage initialized at %1").arg(QString::fromStdString(m_ideRoot.string())));
}

void WorkspaceManager::saveWorkspace(const WorkspaceState& state) {
    if (m_ideRoot.empty()) return;

    std::filesystem::path file = m_ideRoot / "workspace/workspace.json";
    try {
        nlohmann::json j;
        j["version"] = state.version;
        j["activeFile"] = state.activeFile;
        j["openFiles"] = state.openFiles;
        j["buildConfiguration"] = state.buildConfiguration;
        j["platform"] = state.platform;
        j["target"] = state.target;

        nlohmann::json cursors = nlohmann::json::object();
        for (const auto& [f, pos] : state.cursorPositions) {
            cursors[f] = {{"line", pos.line}, {"column", pos.column}};
        }
        j["cursorPositions"] = cursors;

        std::ofstream outFile(file);
        outFile << j.dump(4);

        if (std::filesystem::exists(file) && std::filesystem::file_size(file) > 0) {
            OpenIDE::Core::Logger::instance().info("WorkspaceManager", QString("[Storage] Saved workspace state to %1 (%2 bytes)")
                .arg(QString::fromStdString(file.string()))
                .arg(std::filesystem::file_size(file)));
        } else {
            OpenIDE::Core::Logger::instance().error("WorkspaceManager", QString("[Storage] Failed file verification for %1").arg(QString::fromStdString(file.string())));
        }
    } catch (const std::exception& e) {
        OpenIDE::Core::Logger::instance().error("WorkspaceManager", QString("Failed to save workspace.json: %1").arg(e.what()));
    }
}

WorkspaceState WorkspaceManager::loadWorkspace() {
    WorkspaceState state;
    if (m_ideRoot.empty()) return state;

    std::filesystem::path file = m_ideRoot / "workspace/workspace.json";
    if (!std::filesystem::exists(file)) return state;

    try {
        std::ifstream inFile(file);
        nlohmann::json j;
        inFile >> j;

        if (j.contains("activeFile")) state.activeFile = j["activeFile"];
        if (j.contains("openFiles")) state.openFiles = j["openFiles"].get<std::vector<std::string>>();
        if (j.contains("buildConfiguration")) state.buildConfiguration = j["buildConfiguration"];
        if (j.contains("platform")) state.platform = j["platform"];
        if (j.contains("target")) state.target = j["target"];

        if (j.contains("cursorPositions") && j["cursorPositions"].is_object()) {
            for (auto& [k, v] : j["cursorPositions"].items()) {
                CursorPos cp;
                cp.line = v.value("line", 1);
                cp.column = v.value("column", 1);
                state.cursorPositions[k] = cp;
            }
        }

        OpenIDE::Core::Logger::instance().info("WorkspaceManager", QString("[Storage] Loaded workspace state from %1 (Restored %2 open files)")
            .arg(QString::fromStdString(file.string()))
            .arg(state.openFiles.size()));
    } catch (const std::exception& e) {
        OpenIDE::Core::Logger::instance().error("WorkspaceManager", QString("Failed to load workspace.json: %1").arg(e.what()));
    }

    return state;
}

void WorkspaceManager::saveProjectState(const std::string& buildConfig, const std::string& platform, const std::string& target) {
    if (m_ideRoot.empty()) return;

    std::filesystem::path file = m_ideRoot / "state/project.json";
    try {
        nlohmann::json j = {
            {"buildConfiguration", buildConfig},
            {"platform", platform},
            {"target", target},
            {"lastSaved", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString()}
        };
        std::ofstream outFile(file);
        outFile << j.dump(4);
        OpenIDE::Core::Logger::instance().info("WorkspaceManager", QString("[Storage] Saved project state to %1").arg(QString::fromStdString(file.string())));
    } catch (...) {}
}

void WorkspaceManager::saveSymbolIndex(const std::vector<std::string>& files, int totalSymbols) {
    if (m_ideRoot.empty()) return;

    std::filesystem::path file = m_ideRoot / "index/symbols.json";
    try {
        nlohmann::json j = {
            {"filesCount", files.size()},
            {"symbolsCount", totalSymbols},
            {"files", files},
            {"lastIndexed", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString()}
        };
        std::ofstream outFile(file);
        outFile << j.dump(4);
        OpenIDE::Core::Logger::instance().info("WorkspaceManager", QString("[Storage] Saved symbol index (%1 files, %2 symbols) to %3")
            .arg(files.size()).arg(totalSymbols).arg(QString::fromStdString(file.string())));
    } catch (...) {}
}

void WorkspaceManager::saveCache(const std::string& key, const std::string& value) {
    if (m_ideRoot.empty()) return;

    std::filesystem::path file = m_ideRoot / "cache/cache.json";
    try {
        nlohmann::json j;
        if (std::filesystem::exists(file)) {
            std::ifstream inFile(file);
            inFile >> j;
        }
        j[key] = value;

        std::ofstream outFile(file);
        outFile << j.dump(4);
        OpenIDE::Core::Logger::instance().debug("WorkspaceManager", QString("[Storage] Updated cache key '%1' in %2").arg(QString::fromStdString(key)).arg(QString::fromStdString(file.string())));
    } catch (...) {}
}

void WorkspaceManager::saveLspConfig(const std::string& clangdPath, const std::string& compileDbDir) {
    if (m_ideRoot.empty()) return;

    std::filesystem::path file = m_ideRoot / "lsp/config.json";
    try {
        nlohmann::json j = {
            {"server", "clangd"},
            {"executable", clangdPath},
            {"compileCommandsDir", compileDbDir},
            {"positionEncoding", "utf-16"}
        };
        std::ofstream outFile(file);
        outFile << j.dump(4);
        OpenIDE::Core::Logger::instance().info("WorkspaceManager", QString("[Storage] Saved LSP config to %1").arg(QString::fromStdString(file.string())));
    } catch (...) {}
}

} // namespace OpenIDE::Project
