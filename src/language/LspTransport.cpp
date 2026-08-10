#include "language/LspTransport.h"
#include "core/Logger.h"

namespace MyIDE::Language {

LspTransport::LspTransport(QObject* parent)
    : QObject(parent) {
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &LspTransport::onReadyRead);
    connect(&m_process, &QProcess::errorOccurred, this, &LspTransport::onErrorOccurred);
}

LspTransport::~LspTransport() {
    stop();
}

bool LspTransport::start(const QString& executable, const QStringList& arguments, const QString& workingDir) {
    if (!workingDir.isEmpty()) {
        m_process.setWorkingDirectory(workingDir);
    }
    m_process.start(executable, arguments);
    if (!m_process.waitForStarted(3000)) {
        MyIDE::Core::Logger::instance().error("LspTransport", QString("Failed to start LSP process: %1").arg(executable));
        return false;
    }
    MyIDE::Core::Logger::instance().info("LspTransport", QString("LSP server started: %1").arg(executable));
    return true;
}

void LspTransport::stop() {
    if (m_process.state() != QProcess::NotRunning) {
        m_process.terminate();
        if (!m_process.waitForFinished(1000)) {
            m_process.kill();
        }
    }
}

void LspTransport::sendJson(const nlohmann::json& json) {
    std::string payload = json.dump();
    std::string message = "Content-Length: " + std::to_string(payload.size()) + "\r\n\r\n" + payload;
    m_process.write(message.c_str(), message.size());
}

void LspTransport::onReadyRead() {
    m_buffer.append(m_process.readAllStandardOutput());

    while (true) {
        int headerEnd = m_buffer.indexOf("\r\n\r\n");
        if (headerEnd == -1) break;

        QByteArray header = m_buffer.left(headerEnd);
        int contentLength = -1;

        for (const auto& line : header.split('\n')) {
            if (line.startsWith("Content-Length:")) {
                contentLength = line.mid(15).trimmed().toInt();
                break;
            }
        }

        if (contentLength == -1) break;

        int totalMessageSize = headerEnd + 4 + contentLength;
        if (m_buffer.size() < totalMessageSize) break;

        QByteArray body = m_buffer.mid(headerEnd + 4, contentLength);
        m_buffer.remove(0, totalMessageSize);

        try {
            nlohmann::json json = nlohmann::json::parse(body.toStdString());
            emit messageReceived(json);
        } catch (const std::exception& e) {
            MyIDE::Core::Logger::instance().error("LspTransport", QString("LSP JSON Parse error: %1").arg(e.what()));
        }
    }
}

void LspTransport::onErrorOccurred(QProcess::ProcessError error) {
    Q_UNUSED(error);
    emit errorOccurred(m_process.errorString());
}

} // namespace MyIDE::Language
