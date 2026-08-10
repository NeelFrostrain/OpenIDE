#include "lsp/LspManager.h"
#include "core/Logger.h"
#include "core/Config.h"
#include <QProcess>
#include <QStandardPaths>
#include <QFileInfo>

namespace MyIDE::Lsp {

LspManager& LspManager::instance() {
    static LspManager s_instance;
    return s_instance;
}

LspManager::LspManager() {
    m_client = new Language::LspClient(this);

    connect(m_client, &Language::LspClient::serverReady, [this]() {
        m_state = ServerState::Ready;
        m_restartAttempts = 0;
        MyIDE::Core::Logger::instance().info("LspManager", "[LSP] clangd initialized and Ready");
        emit stateChanged(m_state);
        emit serverReady();
    });

    connect(m_client, &Language::LspClient::diagnosticsPublished, [this](const std::filesystem::path& path, const std::vector<Editor::Diagnostic>& diagnostics) {
        m_diagnosticsManager.publishDiagnostics(path, diagnostics);
    });
}

LspManager::~LspManager() {
    stopServer();
}

QString LspManager::discoverClangdBinary() const {
    // 1. Configured path
    QString configured = MyIDE::Core::Config::instance().clangdExecutable();
    if (QFileInfo::exists(configured)) {
        return configured;
    }

    // 2. WinGet LLVM clangd path
    QString wingetPath = "C:/Users/neelo/AppData/Local/Microsoft/WinGet/Packages/LLVM.clangd_Microsoft.Winget.Source_8wekyb3d8bbwe/clangd_22.1.6/bin/clangd.exe";
    if (QFileInfo::exists(wingetPath)) {
        return wingetPath;
    }

    // 3. Environment PATH
    QString systemClangd = QStandardPaths::findExecutable("clangd");
    if (!systemClangd.isEmpty()) {
        return systemClangd;
    }

    // 4. Visual Studio LLVM path
    QString vsClangd = "C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/bin/clangd.exe";
    if (QFileInfo::exists(vsClangd)) {
        return vsClangd;
    }

    return "clangd";
}

QString LspManager::stateString() const {
    switch (m_state) {
        case ServerState::Starting: return "Starting...";
        case ServerState::Initializing: return "Initializing...";
        case ServerState::Ready: return "Ready";
        case ServerState::Degraded: return "Degraded";
        case ServerState::Restarting: return "Restarting...";
        case ServerState::Stopped: return "Stopped";
        case ServerState::Failed: return "Failed";
    }
    return "Unknown";
}

bool LspManager::startServer(const Project::ProjectPaths& paths) {
    m_currentPaths = paths;
    m_state = ServerState::Starting;
    emit stateChanged(m_state);

    QString binary = discoverClangdBinary();
    MyIDE::Core::Logger::instance().info("LspManager", QString("[LSP] Starting clangd binary: %1").arg(binary));

    // Capture version for diagnostics
    QProcess verProcess;
    verProcess.start(binary, QStringList() << "--version");
    if (verProcess.waitForFinished(1500)) {
        m_serverVersion = QString::fromUtf8(verProcess.readAllStandardOutput()).trimmed();
        MyIDE::Core::Logger::instance().info("LspManager", QString("[LSP] clangd version: %1").arg(m_serverVersion));
    }

    m_state = ServerState::Initializing;
    emit stateChanged(m_state);

    return m_client->start(binary, paths.root);
}

void LspManager::stopServer() {
    m_requestManager.cancelAll();
    if (m_client) {
        m_client->stop();
    }
    m_state = ServerState::Stopped;
    emit stateChanged(m_state);
}

void LspManager::restartServer() {
    if (m_restartAttempts >= 3) {
        m_state = ServerState::Degraded;
        emit stateChanged(m_state);
        MyIDE::Core::Logger::instance().error("LspManager", "[LSP] Max restart attempts reached. Entering Degraded state.");
        return;
    }

    m_restartAttempts++;
    m_state = ServerState::Restarting;
    emit stateChanged(m_state);
    MyIDE::Core::Logger::instance().warn("LspManager", QString("[LSP] Restarting clangd (Attempt %1/3)").arg(m_restartAttempts));

    stopServer();
    startServer(m_currentPaths);
}

void LspManager::requestHover(const std::filesystem::path& path, int line, int col, std::function<void(const HoverInfo&)> callback) {
    if (m_state != ServerState::Ready) return;
    uint64_t gen = m_requestManager.nextGeneration();

    m_client->requestHover(path, line, col, [this, gen, callback](const QString& hoverText) {
        if (gen != m_requestManager.currentGeneration()) return;
        HoverInfo info;
        info.documentation = hoverText;
        if (callback) callback(info);
    });
}

void LspManager::requestDefinition(const std::filesystem::path& path, int line, int col, std::function<void(const std::vector<Location>&)> callback) {
    if (m_state != ServerState::Ready) return;
    uint64_t gen = m_requestManager.nextGeneration();

    m_client->requestDefinition(path, line, col, [this, gen, callback](const std::vector<Language::LocationResult>& results) {
        if (gen != m_requestManager.currentGeneration()) return;
        std::vector<Location> locs;
        for (const auto& r : results) {
            Location loc;
            loc.path = r.path;
            loc.range.start.line = r.line - 1;
            loc.range.start.character = r.column;
            locs.push_back(loc);
        }
        if (callback) callback(locs);
    });
}

void LspManager::requestReferences(const std::filesystem::path& path, int line, int col, std::function<void(const std::vector<Location>&)> callback) {
    if (m_state != ServerState::Ready) return;
    uint64_t gen = m_requestManager.nextGeneration();

    m_client->requestReferences(path, line, col, [this, gen, callback](const std::vector<Language::LocationResult>& results) {
        if (gen != m_requestManager.currentGeneration()) return;
        std::vector<Location> locs;
        for (const auto& r : results) {
            Location loc;
            loc.path = r.path;
            loc.range.start.line = r.line - 1;
            loc.range.start.character = r.column;
            locs.push_back(loc);
        }
        if (callback) callback(locs);
    });
}

void LspManager::requestRename(const std::filesystem::path& path, int line, int col, const QString& newName, std::function<void(bool success)> callback) {
    Q_UNUSED(path); Q_UNUSED(line); Q_UNUSED(col); Q_UNUSED(newName);
    if (callback) callback(true);
}

void LspManager::requestSignatureHelp(const std::filesystem::path& path, int line, int col, std::function<void(const SignatureInfo&)> callback) {
    if (m_state != ServerState::Ready) return;
    uint64_t gen = m_requestManager.nextGeneration();

    m_client->requestSignatureHelp(path, line, col, [this, gen, callback](const Language::SignatureInfo& sigInfo) {
        if (gen != m_requestManager.currentGeneration()) return;
        SignatureInfo info;
        info.label = sigInfo.label;
        info.documentation = sigInfo.documentation;
        if (callback) callback(info);
    });
}

void LspManager::requestDocumentSymbols(const std::filesystem::path& path, std::function<void(const std::vector<DocumentSymbolInfo>&)> callback) {
    Q_UNUSED(path);
    if (callback) callback({});
}

} // namespace MyIDE::Lsp
