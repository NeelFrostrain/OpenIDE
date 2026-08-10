#pragma once

#include "editor/ITextEditor.h"
#include "lsp/LspTypes.h"
#include <QObject>
#include <QTimer>
#include <unordered_map>
#include <vector>
#include <filesystem>

namespace MyIDE::Lsp {

class LspDiagnosticsManager : public QObject {
    Q_OBJECT

public:
    explicit LspDiagnosticsManager(QObject* parent = nullptr);

    void publishDiagnostics(const std::filesystem::path& path, const std::vector<Editor::Diagnostic>& diagnostics);
    std::vector<Editor::Diagnostic> diagnosticsForFile(const std::filesystem::path& path) const;
    void clearDiagnostics();

    int totalErrors() const;
    int totalWarnings() const;

signals:
    void diagnosticsUpdated(const std::filesystem::path& path, const std::vector<Editor::Diagnostic>& diagnostics);

private:
    std::unordered_map<std::string, std::vector<Editor::Diagnostic>> m_fileDiagnostics;
};

} // namespace MyIDE::Lsp
