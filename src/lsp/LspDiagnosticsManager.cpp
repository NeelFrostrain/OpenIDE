#include "lsp/LspDiagnosticsManager.h"
#include "core/Logger.h"

namespace MyIDE::Lsp {

LspDiagnosticsManager::LspDiagnosticsManager(QObject* parent)
    : QObject(parent) {
}

void LspDiagnosticsManager::publishDiagnostics(const std::filesystem::path& path, const std::vector<Editor::Diagnostic>& diagnostics) {
    m_fileDiagnostics[path.string()] = diagnostics;
    emit diagnosticsUpdated(path, diagnostics);

    MyIDE::Core::Logger::instance().info("LspDiagnostics", QString("[LSP] Published %1 diagnostics for %2")
        .arg(diagnostics.size())
        .arg(QString::fromStdString(path.filename().string())));
}

std::vector<Editor::Diagnostic> LspDiagnosticsManager::diagnosticsForFile(const std::filesystem::path& path) const {
    auto it = m_fileDiagnostics.find(path.string());
    if (it != m_fileDiagnostics.end()) {
        return it->second;
    }
    return {};
}

void LspDiagnosticsManager::clearDiagnostics() {
    m_fileDiagnostics.clear();
}

int LspDiagnosticsManager::totalErrors() const {
    int count = 0;
    for (const auto& [p, diagList] : m_fileDiagnostics) {
        for (const auto& d : diagList) {
            if (d.severity == 1) count++;
        }
    }
    return count;
}

int LspDiagnosticsManager::totalWarnings() const {
    int count = 0;
    for (const auto& [p, diagList] : m_fileDiagnostics) {
        for (const auto& d : diagList) {
            if (d.severity == 2) count++;
        }
    }
    return count;
}

} // namespace MyIDE::Lsp
