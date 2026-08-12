#include "editor/Document.h"
#include "core/Logger.h"
#include <QFile>
#include <QTextStream>

namespace OpenIDE::Editor {

Document::Document(DocumentId id, const std::filesystem::path& path, const QString& content)
    : m_id(id), m_path(path), m_content(content) {
}

QString Document::fileName() const {
    return QString::fromStdString(m_path.filename().string());
}

void Document::setContent(const QString& content) {
    if (m_content != content) {
        m_content = content;
        m_version++;
        if (!m_isDirty) {
            m_isDirty = true;
            emit dirtyStateChanged(true);
        }
        emit contentChanged(m_content, m_version);
    }
}

bool Document::save() {
    if (m_path.empty()) return false;
    
    QFile file(QString::fromStdString(m_path.string()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        OpenIDE::Core::Logger::instance().error("Document", QString("Failed to save file: %1").arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out << m_content;
    file.close();

    m_isDirty = false;
    emit dirtyStateChanged(false);
    emit saved();
    OpenIDE::Core::Logger::instance().info("Document", QString("Saved document: %1").arg(fileName()));
    return true;
}

bool Document::reload() {
    if (m_path.empty()) return false;

    QFile file(QString::fromStdString(m_path.string()));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream in(&file);
    m_content = in.readAll();
    file.close();

    m_isDirty = false;
    m_version++;
    emit dirtyStateChanged(false);
    emit contentChanged(m_content, m_version);
    return true;
}

} // namespace OpenIDE::Editor
