#pragma once

#include <QObject>
#include <QProcess>
#include <QByteArray>
#include <nlohmann/json.hpp>

namespace OpenIDE::Language {

class LspTransport : public QObject {
    Q_OBJECT

public:
    explicit LspTransport(QObject* parent = nullptr);
    ~LspTransport() override;

    bool start(const QString& executable, const QStringList& arguments, const QString& workingDir);
    void stop();

    void sendJson(const nlohmann::json& json);

signals:
    void messageReceived(const nlohmann::json& json);
    void errorOccurred(const QString& error);

private slots:
    void onReadyRead();
    void onErrorOccurred(QProcess::ProcessError error);

private:
    QProcess m_process;
    QByteArray m_buffer;
};

} // namespace OpenIDE::Language
