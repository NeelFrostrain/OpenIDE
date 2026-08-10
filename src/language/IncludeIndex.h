#pragma once

#include "editor/ITextEditor.h"
#include <vector>
#include <string>
#include <filesystem>
#include <QString>
#include <mutex>

namespace MyIDE::Language {

class IncludeIndex {
public:
    static IncludeIndex& instance();

    void clearProjectHeaders();
    void addProjectHeader(const std::string& relativePath);
    void removeProjectHeader(const std::string& relativePath);

    bool loadFromDisk(const std::filesystem::path& ideDir);
    void saveToDisk(const std::filesystem::path& ideDir) const;

    std::vector<Editor::CompletionItemData> getIncludeCompletions(const QString& prefix, bool isSystem) const;

private:
    IncludeIndex();

    std::vector<std::string> m_systemHeaders;
    std::vector<std::string> m_projectHeaders;
    mutable std::mutex m_mutex;
};

} // namespace MyIDE::Language
