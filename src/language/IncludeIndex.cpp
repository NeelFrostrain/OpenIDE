#include "language/IncludeIndex.h"
#include "core/Logger.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

namespace OpenIDE::Language {

IncludeIndex& IncludeIndex::instance() {
    static IncludeIndex s_instance;
    return s_instance;
}

IncludeIndex::IncludeIndex() {
    // Standard system headers
    m_systemHeaders = {
        "vector", "string", "memory", "iostream", "algorithm", "map", "unordered_map",
        "set", "unordered_set", "tuple", "utility", "type_traits", "thread", "mutex",
        "chrono", "filesystem", "fstream", "sstream", "queue", "stack", "deque",
        "list", "array", "variant", "optional", "any", "cstdint", "cstdio", "cstdlib",
        "cstring", "cmath", "cassert", "exception", "stdexcept", "functional", "numeric"
    };

    // No hardcoded project headers!
    m_projectHeaders.clear();
}

void IncludeIndex::clearProjectHeaders() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_projectHeaders.clear();
    OpenIDE::Core::Logger::instance().info("IncludeIndex", "[IncludeIndex] Cleared project headers for new project context");
}

void IncludeIndex::addProjectHeader(const std::string& relativePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (std::find(m_projectHeaders.begin(), m_projectHeaders.end(), relativePath) == m_projectHeaders.end()) {
        m_projectHeaders.push_back(relativePath);
    }
}

void IncludeIndex::removeProjectHeader(const std::string& relativePath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::remove(m_projectHeaders.begin(), m_projectHeaders.end(), relativePath);
    if (it != m_projectHeaders.end()) {
        m_projectHeaders.erase(it, m_projectHeaders.end());
    }
}

bool IncludeIndex::loadFromDisk(const std::filesystem::path& ideDir) {
    std::filesystem::path file = ideDir / "index" / "includes.json";
    if (!std::filesystem::exists(file)) return false;

    try {
        std::ifstream inFile(file);
        nlohmann::json j;
        inFile >> j;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_projectHeaders = j.value("headers", std::vector<std::string>{});

        OpenIDE::Core::Logger::instance().info("IncludeIndex", QString("[IncludeIndex] Loaded %1 dynamic project headers from %2")
            .arg(m_projectHeaders.size()).arg(QString::fromStdString(file.string())));
        return true;
    } catch (...) {
        return false;
    }
}

void IncludeIndex::saveToDisk(const std::filesystem::path& ideDir) const {
    try {
        std::filesystem::create_directories(ideDir / "index");
        std::filesystem::path file = ideDir / "index" / "includes.json";

        std::lock_guard<std::mutex> lock(m_mutex);
        nlohmann::json j = {
            {"headers", m_projectHeaders},
            {"count", m_projectHeaders.size()}
        };

        std::ofstream outFile(file);
        outFile << j.dump(4);
    } catch (...) {
    }
}

std::vector<Editor::CompletionItemData> IncludeIndex::getIncludeCompletions(const QString& prefix, bool isSystem) const {
    std::vector<Editor::CompletionItemData> results;
    std::lock_guard<std::mutex> lock(m_mutex);

    const auto& sourceList = isSystem ? m_systemHeaders : m_projectHeaders;

    for (const auto& h : sourceList) {
        QString hStr = QString::fromStdString(h);
        if (prefix.isEmpty() || hStr.startsWith(prefix, Qt::CaseInsensitive) || (prefix.length() >= 2 && hStr.contains(prefix, Qt::CaseInsensitive))) {
            Editor::CompletionItemData item;
            item.label = hStr;
            item.detail = isSystem ? "system header" : "header file";
            item.insertText = hStr;
            item.kind = 17; // File icon
            item.source = Editor::CompletionSource::ProjectIndex;
            results.push_back(item);
        }
    }

    return results;
}

} // namespace OpenIDE::Language
