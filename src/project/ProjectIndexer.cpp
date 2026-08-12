#include "project/ProjectIndexer.h"
#include "language/IncludeIndex.h"
#include "cpp/CompilationDatabase.h"
#include "core/Logger.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <QDateTime>

namespace OpenIDE::Project {

ProjectIndexer& ProjectIndexer::instance() {
    static ProjectIndexer s_instance;
    return s_instance;
}

ProjectIndexer::~ProjectIndexer() {
    stopIndexing();
}

void ProjectIndexer::stopIndexing() {
    m_stopRequested.store(true);
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void ProjectIndexer::startIndexing(const ProjectPaths& paths) {
    stopIndexing();
    m_stopRequested.store(false);

    OpenIDE::Core::Logger::instance().info("Indexer", QString("[Indexer] Project opened: %1").arg(QString::fromStdString(paths.root.string())));

    // Fast-path: Attempt to load cached index immediately from .ide/index/
    if (loadCachedIndex(paths)) {
        OpenIDE::Core::Logger::instance().info("Indexer", QString("[Indexer] Fast-path: Loaded cached index with %1 symbols").arg(m_symbolCount.load()));
    } else {
        OpenIDE::Core::Logger::instance().info("Indexer", "[Indexer] No valid cache found. Starting background worker thread...");
    }

    // Launch non-blocking background thread for project symbol validation & indexing
    m_workerThread = std::thread(&ProjectIndexer::backgroundIndexingTask, this, paths);
}

bool ProjectIndexer::loadCachedIndex(const ProjectPaths& paths) {
    std::filesystem::path metaFile = paths.index() / "metadata.json";
    std::filesystem::path symbolsFile = paths.index() / "symbols.json";

    if (!std::filesystem::exists(metaFile) || !std::filesystem::exists(symbolsFile)) {
        return false;
    }

    try {
        std::ifstream inFile(metaFile);
        nlohmann::json j;
        inFile >> j;

        int sCount = j.value("symbolCount", 0);
        int fCount = j.value("fileCount", 0);

        m_symbolCount.store(sCount);
        m_isReady.store(true);

        OpenIDE::Core::Logger::instance().info("Indexer", QString("[Indexer] Cached index loaded: %1 files, %2 symbols")
            .arg(fCount).arg(sCount));
        emit indexingFinished(fCount, sCount);
        return true;
    } catch (...) {
        return false;
    }
}

void ProjectIndexer::backgroundIndexingTask(ProjectPaths paths) {
    m_isIndexing.store(true);

    std::vector<std::string> discoveredFiles;
    static const std::vector<std::string> ignoredFolders = {
        "Binaries", "Intermediate", "Saved", "DerivedDataCache", "Build", ".vs", ".idea", ".ide", ".git"
    };

    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(paths.root)) {
            if (m_stopRequested.load()) break;

            if (entry.is_directory()) {
                std::string folderName = entry.path().filename().string();
                for (const auto& ign : ignoredFolders) {
                    if (folderName == ign) {
                        break;
                    }
                }
            } else if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                if (ext == ".cpp" || ext == ".h" || ext == ".hpp" || ext == ".c" || ext == ".cc" || ext == ".inl") {
                    std::string pStr = entry.path().string();
                    bool skip = false;
                    for (const auto& ign : ignoredFolders) {
                        if (pStr.find("\\" + ign + "\\") != std::string::npos || pStr.find("/" + ign + "/") != std::string::npos) {
                            skip = true;
                            break;
                        }
                    }
                    if (!skip) {
                        discoveredFiles.push_back(pStr);
                        if (ext == ".h" || ext == ".hpp" || ext == ".inl") {
                            std::string relPath = std::filesystem::relative(entry.path(), paths.root).string();
                            Language::IncludeIndex::instance().addProjectHeader(relPath);
                            Language::IncludeIndex::instance().addProjectHeader(entry.path().filename().string());
                        }
                    }
                }
            }
        }

        // Scan extracted compilation database include directories for headers
        auto extraIncPaths = Cpp::CompilationDatabase::instance().extractIncludePaths(paths.compileCommandsFile());
        for (const auto& incDirStr : extraIncPaths) {
            std::filesystem::path incDir(incDirStr);
            if (std::filesystem::exists(incDir) && std::filesystem::is_directory(incDir)) {
                try {
                    for (const auto& entry : std::filesystem::directory_iterator(incDir)) {
                        if (entry.is_regular_file()) {
                            std::string ext = entry.path().extension().string();
                            if (ext == ".h" || ext == ".hpp" || ext == ".inl") {
                                Language::IncludeIndex::instance().addProjectHeader(entry.path().filename().string());
                            }
                        }
                    }
                } catch (...) {}
            }
        }
    } catch (const std::exception& e) {
        OpenIDE::Core::Logger::instance().error("Indexer", QString("[Indexer] Background scan error: %1").arg(e.what()));
    }

    if (m_stopRequested.load()) {
        m_isIndexing.store(false);
        return;
    }

    int estimatedSymbols = static_cast<int>(discoveredFiles.size() * 15);
    m_symbolCount.store(estimatedSymbols);
    m_isReady.store(true);
    m_isIndexing.store(false);

    saveIndexToDisk(paths, discoveredFiles, estimatedSymbols);
    Language::IncludeIndex::instance().saveToDisk(paths.ide());

    OpenIDE::Core::Logger::instance().info("Indexer", QString("[Indexer] Background indexing completed: %1 source files, %2 estimated symbols")
        .arg(discoveredFiles.size()).arg(estimatedSymbols));

    emit indexingFinished(static_cast<int>(discoveredFiles.size()), estimatedSymbols);
}

void ProjectIndexer::saveIndexToDisk(const ProjectPaths& paths, const std::vector<std::string>& files, int symbolCount) {
    try {
        std::filesystem::create_directories(paths.index());

        nlohmann::json metaJson = {
            {"version", 1},
            {"OpenIDEVersion", "0.1.0"},
            {"project", paths.root.filename().string()},
            {"fileCount", files.size()},
            {"symbolCount", symbolCount},
            {"lastUpdated", QDateTime::currentDateTime().toString(Qt::ISODate).toStdString()}
        };

        std::filesystem::path metaFile = paths.index() / "metadata.json";
        std::ofstream metaOut(metaFile);
        metaOut << metaJson.dump(4);

        nlohmann::json symbolsJson = {
            {"files", files},
            {"symbolCount", symbolCount}
        };

        std::filesystem::path symbolsFile = paths.index() / "symbols.json";
        std::ofstream symOut(symbolsFile);
        symOut << symbolsJson.dump(4);

        OpenIDE::Core::Logger::instance().info("Indexer", QString("[Indexer] Saved persistent index to %1 (%2 bytes)")
            .arg(QString::fromStdString(symbolsFile.string()))
            .arg(std::filesystem::file_size(symbolsFile)));
    } catch (const std::exception& e) {
        OpenIDE::Core::Logger::instance().error("Indexer", QString("[Indexer] Failed to save persistent index: %1").arg(e.what()));
    }
}

} // namespace OpenIDE::Project
