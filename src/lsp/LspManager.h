#pragma once

#include "lsp/LspTypes.h"
#include "lsp/LspRequestManager.h"
#include "lsp/LspDiagnosticsManager.h"
#include "lsp/LspSemanticTokenManager.h"
#include "language/LspClient.h"
#include "project/ProjectManager.h"
#include <QObject>
#include <filesystem>
#include <memory>
#include <string>

namespace OpenIDE::Lsp {

enum class ServerState {
    Starting,
    Initializing,
    Ready,
    Degraded,
    Restarting,
    Stopped,
    Failed
};

class LspManager : public QObject {
    Q_OBJECT

public:
    static LspManager& instance();

    bool startServer(const Project::ProjectPaths& paths);
    void stopServer();
    void restartServer();

    ServerState state() const { return m_state; }
    QString stateString() const;
    QString serverVersion() const { return m_serverVersion; }

    Language::LspClient* client() { return m_client; }
    LspRequestManager* requestManager() { return &m_requestManager; }
    LspDiagnosticsManager* diagnosticsManager() { return &m_diagnosticsManager; }
    LspSemanticTokenManager* semanticTokenManager() { return &m_semanticTokenManager; }

    // Hover, Definition, References, Rename, SignatureHelp, DocumentSymbols
    void requestHover(const std::filesystem::path& path, int line, int col, std::function<void(const HoverInfo&)> callback);
    void requestDefinition(const std::filesystem::path& path, int line, int col, std::function<void(const std::vector<Location>&)> callback);
    void requestReferences(const std::filesystem::path& path, int line, int col, std::function<void(const std::vector<Location>&)> callback);
    void requestRename(const std::filesystem::path& path, int line, int col, const QString& newName, std::function<void(bool success)> callback);
    void requestSignatureHelp(const std::filesystem::path& path, int line, int col, std::function<void(const SignatureInfo&)> callback);
    void requestDocumentSymbols(const std::filesystem::path& path, std::function<void(const std::vector<DocumentSymbolInfo>&)> callback);

    QString discoverClangdBinary() const;

signals:
    void stateChanged(ServerState newState);
    void serverReady();

private:
    LspManager();
    ~LspManager();

    ServerState m_state = ServerState::Stopped;
    Project::ProjectPaths m_currentPaths;
    QString m_serverVersion;
    int m_restartAttempts = 0;

    Language::LspClient* m_client = nullptr;
    LspRequestManager m_requestManager;
    LspDiagnosticsManager m_diagnosticsManager;
    LspSemanticTokenManager m_semanticTokenManager;
};

} // namespace OpenIDE::Lsp
