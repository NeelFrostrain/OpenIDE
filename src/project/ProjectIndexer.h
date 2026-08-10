#pragma once

#include "project/ProjectManager.h"
#include <QObject>
#include <filesystem>
#include <vector>
#include <atomic>
#include <thread>
#include <mutex>

namespace MyIDE::Project {

struct IndexMetadata {
    int version = 1;
    std::string project;
    int fileCount = 0;
    int symbolCount = 0;
    std::string lastUpdated;
};

class ProjectIndexer : public QObject {
    Q_OBJECT

public:
    static ProjectIndexer& instance();
    ~ProjectIndexer();

    void startIndexing(const ProjectPaths& paths);
    void stopIndexing();

    bool isReady() const { return m_isReady.load(); }
    int symbolCount() const { return m_symbolCount.load(); }

signals:
    void indexingProgress(int percent, int totalFiles, int totalSymbols);
    void indexingFinished(int totalFiles, int totalSymbols);

private:
    ProjectIndexer() = default;

    void backgroundIndexingTask(ProjectPaths paths);
    bool loadCachedIndex(const ProjectPaths& paths);
    void saveIndexToDisk(const ProjectPaths& paths, const std::vector<std::string>& files, int symbolCount);

    std::atomic<bool> m_isReady{false};
    std::atomic<bool> m_isIndexing{false};
    std::atomic<bool> m_stopRequested{false};
    std::atomic<int> m_symbolCount{0};

    std::thread m_workerThread;
    std::mutex m_mutex;
};

} // namespace MyIDE::Project
