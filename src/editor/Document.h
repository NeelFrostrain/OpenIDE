#pragma once

#include <QString>
#include <QObject>
#include <filesystem>
#include <cstdint>

namespace OpenIDE::Editor {

using DocumentId = std::uint64_t;

class Document : public QObject {
    Q_OBJECT

public:
    Document(DocumentId id, const std::filesystem::path& path, const QString& content);

    DocumentId id() const { return m_id; }
    std::filesystem::path path() const { return m_path; }
    QString fileName() const;
    QString content() const { return m_content; }
    void setContent(const QString& content);

    bool isDirty() const { return m_isDirty; }
    int version() const { return m_version; }

    bool save();
    bool reload();

signals:
    void contentChanged(const QString& newContent, int version);
    void dirtyStateChanged(bool isDirty);
    void saved();

private:
    DocumentId m_id;
    std::filesystem::path m_path;
    QString m_content;
    bool m_isDirty = false;
    int m_version = 0;
};

} // namespace OpenIDE::Editor
