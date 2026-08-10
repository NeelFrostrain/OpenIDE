#include "LspClient.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QUrl>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>

LspClient::LspClient(QObject* parent) : QObject(parent) {
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &LspClient::handleReadyRead);
    connect(&m_process, &QProcess::finished, this, [this] { emit serverStopped(); });
    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError err) {
        emit serverError(QString("Process error code %1: %2").arg(err).arg(m_process.errorString()));
    });
}

LspClient::~LspClient() {
    stopServer();
}

bool LspClient::startServer(const QString& executable, const QStringList& args, const QString& workingDir) {
    if (m_process.state() != QProcess::NotRunning) {
        stopServer();
    }
    m_process.setWorkingDirectory(workingDir);
    m_process.start(executable, args);
    bool started = m_process.waitForStarted(3000);
    if (started) {
        emit serverStarted();
    } else {
        emit serverError(QString("Failed to start executable: %1").arg(executable));
    }
    return started;
}

void LspClient::stopServer() {
    if (m_process.state() != QProcess::NotRunning) {
        m_process.terminate();
        if (!m_process.waitForFinished(2000)) {
            m_process.kill();
        }
    }
}

bool LspClient::isRunning() const {
    return m_process.state() == QProcess::Running;
}

QString LspClient::pathToUri(const QString& filePath) const {
    return QUrl::fromLocalFile(filePath).toString();
}

QString LspClient::uriToPath(const QString& uri) const {
    return QUrl(uri).toLocalFile();
}

void LspClient::sendJsonRpc(const QJsonObject& json) {
    QJsonDocument doc(json);
    QByteArray body = doc.toJson(QJsonDocument::Compact);
    QByteArray header = QString("Content-Length: %1\r\n\r\n").arg(body.size()).toUtf8();
    m_process.write(header);
    m_process.write(body);
}

int LspClient::initialize(const QString& rootPath) {
    int reqId = m_nextRequestId++;
    m_pendingRequests[reqId] = "initialize";

    QJsonObject params;
    params["processId"] = static_cast<int>(QCoreApplication::applicationPid());
    params["rootUri"] = pathToUri(rootPath);
    params["capabilities"] = QJsonObject{
        {"textDocument", QJsonObject{
            {"completion", QJsonObject{
                {"completionItem", QJsonObject{{"snippetSupport", false}}}
            }},
            {"hover", QJsonObject{}},
            {"definition", QJsonObject{}},
            {"publishDiagnostics", QJsonObject{}}
        }}
    };

    QJsonObject req;
    req["jsonrpc"] = "2.0";
    req["id"] = reqId;
    req["method"] = "initialize";
    req["params"] = params;

    sendJsonRpc(req);
    return reqId;
}

void LspClient::sendInitializedNotification() {
    QJsonObject notif;
    notif["jsonrpc"] = "2.0";
    notif["method"] = "initialized";
    notif["params"] = QJsonObject{};
    sendJsonRpc(notif);
}

void LspClient::didOpen(const QString& filePath, const QString& text) {
    QJsonObject item;
    item["uri"] = pathToUri(filePath);
    item["languageId"] = "cpp";
    item["version"] = m_documentVersion++;
    item["text"] = text;

    QJsonObject params;
    params["textDocument"] = item;

    QJsonObject notif;
    notif["jsonrpc"] = "2.0";
    notif["method"] = "textDocument/didOpen";
    notif["params"] = params;

    sendJsonRpc(notif);
}

void LspClient::didChange(const QString& filePath, const QString& text) {
    QJsonObject doc;
    doc["uri"] = pathToUri(filePath);
    doc["version"] = m_documentVersion++;

    QJsonObject contentChange;
    contentChange["text"] = text;

    QJsonArray changes;
    changes.append(contentChange);

    QJsonObject params;
    params["textDocument"] = doc;
    params["contentChanges"] = changes;

    QJsonObject notif;
    notif["jsonrpc"] = "2.0";
    notif["method"] = "textDocument/didChange";
    notif["params"] = params;

    sendJsonRpc(notif);
}

int LspClient::requestCompletion(const QString& filePath, int line, int character) {
    int reqId = m_nextRequestId++;
    m_pendingRequests[reqId] = "completion";

    QJsonObject pos;
    pos["line"] = line;
    pos["character"] = character;

    QJsonObject params;
    params["textDocument"] = QJsonObject{{"uri", pathToUri(filePath)}};
    params["position"] = pos;

    QJsonObject req;
    req["jsonrpc"] = "2.0";
    req["id"] = reqId;
    req["method"] = "textDocument/completion";
    req["params"] = params;

    sendJsonRpc(req);
    return reqId;
}

int LspClient::requestHover(const QString& filePath, int line, int character) {
    int reqId = m_nextRequestId++;
    m_pendingRequests[reqId] = "hover";

    QJsonObject pos;
    pos["line"] = line;
    pos["character"] = character;

    QJsonObject params;
    params["textDocument"] = QJsonObject{{"uri", pathToUri(filePath)}};
    params["position"] = pos;

    QJsonObject req;
    req["jsonrpc"] = "2.0";
    req["id"] = reqId;
    req["method"] = "textDocument/hover";
    req["params"] = params;

    sendJsonRpc(req);
    return reqId;
}

int LspClient::requestDefinition(const QString& filePath, int line, int character) {
    int reqId = m_nextRequestId++;
    m_pendingRequests[reqId] = "definition";

    QJsonObject pos;
    pos["line"] = line;
    pos["character"] = character;

    QJsonObject params;
    params["textDocument"] = QJsonObject{{"uri", pathToUri(filePath)}};
    params["position"] = pos;

    QJsonObject req;
    req["jsonrpc"] = "2.0";
    req["id"] = reqId;
    req["method"] = "textDocument/definition";
    req["params"] = params;

    sendJsonRpc(req);
    return reqId;
}

void LspClient::handleReadyRead() {
    m_readBuffer.append(m_process.readAllStandardOutput());

    while (true) {
        int headerEnd = m_readBuffer.indexOf("\r\n\r\n");
        if (headerEnd == -1) break;

        QByteArray header = m_readBuffer.left(headerEnd);
        int contentLength = 0;

        const auto lines = header.split('\n');
        for (const auto& line : lines) {
            if (line.startsWith("Content-Length:")) {
                contentLength = line.mid(15).trimmed().toInt();
            }
        }

        int messageStart = headerEnd + 4;
        if (m_readBuffer.size() < messageStart + contentLength) {
            break;
        }

        QByteArray body = m_readBuffer.mid(messageStart, contentLength);
        m_readBuffer.remove(0, messageStart + contentLength);

        QJsonDocument doc = QJsonDocument::fromJson(body);
        if (doc.isObject()) {
            processMessage(doc.object());
        }
    }
}

void LspClient::processMessage(const QJsonObject& json) {
    if (json.contains("method")) {
        QString method = json["method"].toString();
        if (method == "textDocument/publishDiagnostics") {
            QJsonObject params = json["params"].toObject();
            QString uri = params["uri"].toString();
            QString filePath = uriToPath(uri);
            QJsonArray diagArray = params["diagnostics"].toArray();

            QList<LspDiagnostic> diagnostics;
            for (const auto& val : diagArray) {
                QJsonObject dObj = val.toObject();
                LspDiagnostic d;
                d.message = dObj["message"].toString();
                d.severity = static_cast<LspDiagnosticSeverity>(dObj["severity"].toInt(1));
                d.code = dObj["code"].toString();
                d.source = dObj["source"].toString();

                QJsonObject rangeObj = dObj["range"].toObject();
                d.range.start.line = rangeObj["start"].toObject()["line"].toInt();
                d.range.start.character = rangeObj["start"].toObject()["character"].toInt();
                d.range.end.line = rangeObj["end"].toObject()["line"].toInt();
                d.range.end.character = rangeObj["end"].toObject()["character"].toInt();

                diagnostics.append(d);
            }
            emit diagnosticsReceived(filePath, diagnostics);
        }
    } else if (json.contains("id")) {
        int id = json["id"].toInt();
        if (m_pendingRequests.contains(id)) {
            QString reqType = m_pendingRequests.take(id);
            if (reqType == "initialize") {
                sendInitializedNotification();
                emit initialized();
            } else if (reqType == "completion") {
                QList<LspCompletionItem> items;
                QJsonObject res = json["result"].toObject();
                QJsonArray itemArray = res.contains("items") ? res["items"].toArray() : json["result"].toArray();

                for (const auto& val : itemArray) {
                    QJsonObject iObj = val.toObject();
                    LspCompletionItem item;
                    item.label = iObj["label"].toString();
                    item.kind = iObj["kind"].toInt();
                    item.detail = iObj["detail"].toString();
                    item.insertText = iObj.contains("insertText") ? iObj["insertText"].toString() : item.label;
                    items.append(item);
                }
                emit completionReady(id, items);
            } else if (reqType == "hover") {
                QJsonObject res = json["result"].toObject();
                QString contentStr;
                if (res.contains("contents")) {
                    auto c = res["contents"];
                    if (c.isString()) {
                        contentStr = c.toString();
                    } else if (c.isObject()) {
                        contentStr = c.toObject()["value"].toString();
                    }
                }
                emit hoverReady(id, contentStr);
            } else if (reqType == "definition") {
                QString targetPath;
                int targetLine = 0;
                int targetCol = 0;

                auto res = json["result"];
                QJsonObject locObj;
                if (res.isArray() && !res.toArray().isEmpty()) {
                    locObj = res.toArray().at(0).toObject();
                } else if (res.isObject()) {
                    locObj = res.toObject();
                }

                if (!locObj.isEmpty()) {
                    targetPath = uriToPath(locObj["uri"].toString());
                    QJsonObject start = locObj["range"].toObject()["start"].toObject();
                    targetLine = start["line"].toInt();
                    targetCol = start["character"].toInt();
                }
                emit definitionReady(id, targetPath, targetLine, targetCol);
            }
        }
    }
}
