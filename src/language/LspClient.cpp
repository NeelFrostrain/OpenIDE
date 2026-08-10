#include "language/LspClient.h"
#include "core/Logger.h"
#include <QUrl>
#include <QCoreApplication>

namespace MyIDE::Language {

static QString pathToUri(const std::filesystem::path& path) {
    return QUrl::fromLocalFile(QString::fromStdString(path.string())).toString();
}

static std::filesystem::path uriToPath(const QString& uriStr) {
    return QUrl(uriStr).toLocalFile().toStdString();
}

LspClient::LspClient(QObject* parent)
    : QObject(parent) {
    connect(&m_transport, &LspTransport::messageReceived, this, &LspClient::onMessageReceived);
}

LspClient::~LspClient() {
    stop();
}

bool LspClient::start(const QString& clangdPath, const std::filesystem::path& workspaceRoot) {
    m_workspaceRoot = workspaceRoot;
    std::filesystem::path lspDir = workspaceRoot / ".ide" / "lsp";
    QStringList args = {
        QString("--compile-commands-dir=%1").arg(QString::fromStdString(lspDir.string())),
        "--header-insertion=never",
        "--completion-style=detailed",
        "--clang-tidy",
        "-j=4"
    };

    if (!m_transport.start(clangdPath, args, QString::fromStdString(workspaceRoot.string()))) {
        return false;
    }

    // Send initialize request
    int id = m_nextRequestId++;
    nlohmann::json initParams = {
        {"processId", QCoreApplication::applicationPid()},
        {"rootUri", pathToUri(workspaceRoot).toStdString()},
        {"capabilities", {
            {"textDocument", {
                {"completion", {
                    {"completionItem", {{"snippetSupport", true}}}
                }},
                {"definition", {{"linkSupport", true}}},
                {"references", {{"dynamicRegistration", true}}},
                {"signatureHelp", {{"signatureInformation", {{"activeParameterWithSupport", true}}}}},
                {"publishDiagnostics", {{"relatedInformation", true}}}
            }}
        }}
    };

    nlohmann::json req = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"method", "initialize"},
        {"params", initParams}
    };

    m_responseCallbacks[id] = [this](const nlohmann::json& response) {
        Q_UNUSED(response);
        nlohmann::json initializedNotif = {
            {"jsonrpc", "2.0"},
            {"method", "initialized"},
            {"params", nlohmann::json::object()}
        };
        m_transport.sendJson(initializedNotif);
        m_initialized = true;
        
        MyIDE::Core::Logger::instance().info("LspClient", "clangd initialized successfully. Flushing pending open documents...");

        // Send didOpen for all opened documents
        for (const auto& [pathStr, content] : m_openDocuments) {
            nlohmann::json params = {
                {"textDocument", {
                    {"uri", pathToUri(pathStr).toStdString()},
                    {"languageId", "cpp"},
                    {"version", 1},
                    {"text", content.toStdString()}
                }}
            };
            nlohmann::json notif = {
                {"jsonrpc", "2.0"},
                {"method", "textDocument/didOpen"},
                {"params", params}
            };
            m_transport.sendJson(notif);
            MyIDE::Core::Logger::instance().debug("LspClient", QString("Flushed textDocument/didOpen for %1").arg(QString::fromStdString(pathStr)));
        }

        emit serverReady();
    };

    m_transport.sendJson(req);
    return true;
}

void LspClient::stop() {
    if (m_initialized) {
        nlohmann::json req = {
            {"jsonrpc", "2.0"},
            {"id", m_nextRequestId++},
            {"method", "shutdown"}
        };
        m_transport.sendJson(req);
    }
    m_transport.stop();
    m_initialized = false;
    m_openDocuments.clear();
}

void LspClient::didOpen(const std::filesystem::path& path, const QString& content) {
    std::string pathStr = path.string();
    m_openDocuments[pathStr] = content;

    if (!m_initialized) {
        MyIDE::Core::Logger::instance().debug("LspClient", QString("clangd not initialized yet. Queued didOpen for %1").arg(QString::fromStdString(pathStr)));
        return;
    }

    nlohmann::json params = {
        {"textDocument", {
            {"uri", pathToUri(path).toStdString()},
            {"languageId", "cpp"},
            {"version", 1},
            {"text", content.toStdString()}
        }}
    };

    nlohmann::json notif = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didOpen"},
        {"params", params}
    };

    m_transport.sendJson(notif);
    MyIDE::Core::Logger::instance().debug("LspClient", QString("Sent textDocument/didOpen for %1").arg(QString::fromStdString(pathStr)));
}

void LspClient::didChange(const std::filesystem::path& path, const QString& content, int version) {
    if (!m_initialized) return;

    nlohmann::json params = {
        {"textDocument", {
            {"uri", pathToUri(path).toStdString()},
            {"version", version}
        }},
        {"contentChanges", nlohmann::json::array({
            {{"text", content.toStdString()}}
        })}
    };

    nlohmann::json notif = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didChange"},
        {"params", params}
    };

    m_transport.sendJson(notif);
}

void LspClient::didSave(const std::filesystem::path& path) {
    if (!m_initialized) return;

    nlohmann::json params = {
        {"textDocument", {{"uri", pathToUri(path).toStdString()}}}
    };

    nlohmann::json notif = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/didSave"},
        {"params", params}
    };

    m_transport.sendJson(notif);
}

void LspClient::requestCompletion(const std::filesystem::path& path, int line, int column, std::function<void(const std::vector<Editor::CompletionItemData>&)> callback) {
    if (!m_initialized) {
        MyIDE::Core::Logger::instance().info("LspClient", "Cannot request completion: clangd not initialized yet");
        callback({});
        return;
    }

    int id = m_nextRequestId++;
    nlohmann::json params = {
        {"textDocument", {{"uri", pathToUri(path).toStdString()}}},
        {"position", {{"line", line - 1}, {"character", column}}}
    };

    nlohmann::json req = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"method", "textDocument/completion"},
        {"params", params}
    };

    MyIDE::Core::Logger::instance().debug("LspClient", QString("Sending textDocument/completion req #%1 for line=%2 col=%3 uri=%4").arg(id).arg(line - 1).arg(column).arg(pathToUri(path)));

    m_responseCallbacks[id] = [id, callback](const nlohmann::json& response) {
        std::vector<Editor::CompletionItemData> result;

        const nlohmann::json* itemsArray = nullptr;
        if (response.contains("result")) {
            const auto& res = response["result"];
            if (res.is_array()) {
                itemsArray = &res;
            } else if (res.is_object() && res.contains("items") && res["items"].is_array()) {
                itemsArray = &res["items"];
            }
        }

        if (itemsArray) {
            for (const auto& item : *itemsArray) {
                Editor::CompletionItemData data;
                data.label = QString::fromStdString(item.value("label", ""));
                data.detail = QString::fromStdString(item.value("detail", ""));
                data.insertText = QString::fromStdString(item.value("insertText", data.label.toStdString()));
                data.kind = item.value("kind", 0);
                if (item.contains("documentation")) {
                    if (item["documentation"].is_string()) {
                        data.documentation = QString::fromStdString(item["documentation"]);
                    } else if (item["documentation"].is_object() && item["documentation"].contains("value")) {
                        data.documentation = QString::fromStdString(item["documentation"]["value"]);
                    }
                }
                result.push_back(data);
            }
        }

        MyIDE::Core::Logger::instance().info("LspClient", QString("LSP completion req #%1 returned %2 items").arg(id).arg(result.size()));
        callback(result);
    };

    m_transport.sendJson(req);
}

void LspClient::requestDefinition(const std::filesystem::path& path, int line, int column, std::function<void(const std::vector<LocationResult>&)> callback) {
    if (!m_initialized) {
        callback({});
        return;
    }

    int id = m_nextRequestId++;
    nlohmann::json params = {
        {"textDocument", {{"uri", pathToUri(path).toStdString()}}},
        {"position", {{"line", line - 1}, {"character", column}}}
    };

    nlohmann::json req = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"method", "textDocument/definition"},
        {"params", params}
    };

    m_responseCallbacks[id] = [callback](const nlohmann::json& response) {
        std::vector<LocationResult> results;
        if (response.contains("result")) {
            const auto& res = response["result"];
            if (res.is_array()) {
                for (const auto& loc : res) {
                    LocationResult lr;
                    QString uri = QString::fromStdString(loc.value("uri", loc.value("targetUri", "")));
                    lr.path = uriToPath(uri);
                    if (loc.contains("range")) {
                        lr.line = loc["range"]["start"]["line"].get<int>() + 1;
                        lr.column = loc["range"]["start"]["character"].get<int>();
                    }
                    results.push_back(lr);
                }
            } else if (res.is_object()) {
                LocationResult lr;
                lr.path = uriToPath(QString::fromStdString(res.value("uri", res.value("targetUri", ""))));
                if (res.contains("range")) {
                    lr.line = res["range"]["start"]["line"].get<int>() + 1;
                    lr.column = res["range"]["start"]["character"].get<int>();
                }
                results.push_back(lr);
            }
        }
        callback(results);
    };

    m_transport.sendJson(req);
}

void LspClient::requestReferences(const std::filesystem::path& path, int line, int column, std::function<void(const std::vector<LocationResult>&)> callback) {
    if (!m_initialized) {
        callback({});
        return;
    }

    int id = m_nextRequestId++;
    nlohmann::json params = {
        {"textDocument", {{"uri", pathToUri(path).toStdString()}}},
        {"position", {{"line", line - 1}, {"character", column}}},
        {"context", {{"includeDeclaration", true}}}
    };

    nlohmann::json req = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"method", "textDocument/references"},
        {"params", params}
    };

    m_responseCallbacks[id] = [callback](const nlohmann::json& response) {
        std::vector<LocationResult> results;
        if (response.contains("result") && response["result"].is_array()) {
            for (const auto& loc : response["result"]) {
                LocationResult lr;
                lr.path = uriToPath(QString::fromStdString(loc.value("uri", "")));
                if (loc.contains("range")) {
                    lr.line = loc["range"]["start"]["line"].get<int>() + 1;
                    lr.column = loc["range"]["start"]["character"].get<int>();
                }
                results.push_back(lr);
            }
        }
        callback(results);
    };

    m_transport.sendJson(req);
}

void LspClient::requestSignatureHelp(const std::filesystem::path& path, int line, int column, std::function<void(const SignatureInfo&)> callback) {
    if (!m_initialized) {
        callback({});
        return;
    }

    int id = m_nextRequestId++;
    nlohmann::json params = {
        {"textDocument", {{"uri", pathToUri(path).toStdString()}}},
        {"position", {{"line", line - 1}, {"character", column}}}
    };

    nlohmann::json req = {
        {"jsonrpc", "2.0"},
        {"id", id},
        {"method", "textDocument/signatureHelp"},
        {"params", params}
    };

    m_responseCallbacks[id] = [callback](const nlohmann::json& response) {
        SignatureInfo info;
        if (response.contains("result") && response["result"].is_object()) {
            const auto& res = response["result"];
            if (res.contains("signatures") && res["signatures"].is_array() && !res["signatures"].empty()) {
                const auto& sig = res["signatures"][0];
                info.label = QString::fromStdString(sig.value("label", ""));
                info.activeParameter = res.value("activeParameter", 0);

                if (sig.contains("parameters") && sig["parameters"].is_array()) {
                    for (const auto& p : sig["parameters"]) {
                        ParameterInfo pi;
                        pi.label = QString::fromStdString(p.value("label", ""));
                        info.parameters.push_back(pi);
                    }
                }
            }
        }
        callback(info);
    };

    m_transport.sendJson(req);
}

void LspClient::onMessageReceived(const nlohmann::json& json) {
    if (json.contains("id") && !json["id"].is_null()) {
        int id = json["id"];
        auto it = m_responseCallbacks.find(id);
        if (it != m_responseCallbacks.end()) {
            it->second(json);
            m_responseCallbacks.erase(it);
        }
    } else if (json.contains("method")) {
        std::string method = json["method"];
        if (method == "textDocument/publishDiagnostics") {
            const auto& params = json["params"];
            std::filesystem::path path = uriToPath(QString::fromStdString(params["uri"]));

            std::vector<Editor::Diagnostic> diagnostics;
            if (params.contains("diagnostics") && params["diagnostics"].is_array()) {
                for (const auto& diag : params["diagnostics"]) {
                    Editor::Diagnostic d;
                    d.startLine = diag["range"]["start"]["line"].get<int>() + 1;
                    d.startColumn = diag["range"]["start"]["character"].get<int>();
                    d.endLine = diag["range"]["end"]["line"].get<int>() + 1;
                    d.endColumn = diag["range"]["end"]["character"].get<int>();
                    d.message = QString::fromStdString(diag.value("message", ""));
                    d.severity = diag.value("severity", 1);
                    diagnostics.push_back(d);
                }
            }
            emit diagnosticsPublished(path, diagnostics);
        }
    }
}

} // namespace MyIDE::Language
