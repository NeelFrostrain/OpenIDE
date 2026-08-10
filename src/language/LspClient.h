#pragma once

#include "language/LspTransport.h"
#include "editor/ITextEditor.h"
#include <QObject>
#include <filesystem>
#include <unordered_map>
#include <functional>

namespace MyIDE::Language {

struct LocationResult {
    std::filesystem::path path;
    int line = 1;
    int column = 0;
};

struct ParameterInfo {
    QString label;
    QString documentation;
};

struct SignatureInfo {
    QString label;
    QString documentation;
    std::vector<ParameterInfo> parameters;
    int activeParameter = 0;
};

class LspClient : public QObject {
    Q_OBJECT

public:
    explicit LspClient(QObject* parent = nullptr);
    ~LspClient() override;

    bool start(const QString& clangdPath, const std::filesystem::path& workspaceRoot);
    void stop();

    void didOpen(const std::filesystem::path& path, const QString& content);
    void didChange(const std::filesystem::path& path, const QString& content, int version);
    void didSave(const std::filesystem::path& path);

    void requestCompletion(const std::filesystem::path& path, int line, int column, std::function<void(const std::vector<Editor::CompletionItemData>&)> callback);
    void requestHover(const std::filesystem::path& path, int line, int column, std::function<void(const QString&)> callback);
    void requestDefinition(const std::filesystem::path& path, int line, int column, std::function<void(const std::vector<LocationResult>&)> callback);
    void requestReferences(const std::filesystem::path& path, int line, int column, std::function<void(const std::vector<LocationResult>&)> callback);
    void requestSignatureHelp(const std::filesystem::path& path, int line, int column, std::function<void(const SignatureInfo&)> callback);

signals:
    void diagnosticsPublished(const std::filesystem::path& path, const std::vector<Editor::Diagnostic>& diagnostics);
    void serverReady();

private slots:
    void onMessageReceived(const nlohmann::json& json);

private:
    LspTransport m_transport;
    int m_nextRequestId = 1;
    bool m_initialized = false;
    std::filesystem::path m_workspaceRoot;

    std::unordered_map<std::string, QString> m_openDocuments;
    std::unordered_map<int, std::function<void(const nlohmann::json&)>> m_responseCallbacks;
};

} // namespace MyIDE::Language
