#pragma once

#include <QObject>
#include <QProcess>
#include <QByteArray>
#include <QJsonObject>
#include <QMap>
#include "LspTypes.h"

class LspClient : public QObject {
    Q_OBJECT
public:
    explicit LspClient(QObject* parent = nullptr);
    ~LspClient() override;

    bool startServer(const QString& executable, const QStringList& args, const QString& workingDir);
    void stopServer();
    bool isRunning() const;

    int initialize(const QString& rootPath);
    void sendInitializedNotification();

    void didOpen(const QString& filePath, const QString& text);
    void didChange(const QString& filePath, const QString& text);

    int requestCompletion(const QString& filePath, int line, int character);
    int requestHover(const QString& filePath, int line, int character);
    int requestDefinition(const QString& filePath, int line, int character);

signals:
    void serverStarted();
    void serverStopped();
    void serverError(const QString& errorMessage);
    void initialized();
    void diagnosticsReceived(const QString& filePath, const QList<LspDiagnostic>& diagnostics);
    void completionReady(int reqId, const QList<LspCompletionItem>& items);
    void hoverReady(int reqId, const QString& contents);
    void definitionReady(int reqId, const QString& targetPath, int line, int character);

private slots:
    void handleReadyRead();

private:
    void sendJsonRpc(const QJsonObject& json);
    void processMessage(const QJsonObject& json);

    QString pathToUri(const QString& filePath) const;
    QString uriToPath(const QString& uri) const;

    QProcess m_process;
    QByteArray m_readBuffer;
    int m_nextRequestId{1};
    int m_documentVersion{1};
    QMap<int, QString> m_pendingRequests;
};
