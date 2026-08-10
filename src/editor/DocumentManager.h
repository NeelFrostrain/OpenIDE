#pragma once

#include "editor/Document.h"
#include <QObject>
#include <unordered_map>
#include <memory>
#include <filesystem>

namespace MyIDE::Editor {

class DocumentManager : public QObject {
    Q_OBJECT

public:
    static DocumentManager& instance();

    Document* openDocument(const std::filesystem::path& path);
    Document* createUntitledDocument();
    bool closeDocument(DocumentId id);

    Document* getDocument(DocumentId id) const;
    Document* getDocumentByPath(const std::filesystem::path& path) const;

    void saveAll();
    std::vector<Document*> openDocuments() const;

signals:
    void documentOpened(Document* doc);
    void documentClosed(DocumentId id);
    void documentSaved(Document* doc);

private:
    DocumentManager() = default;

    DocumentId m_nextId = 1;
    std::unordered_map<DocumentId, std::unique_ptr<Document>> m_documents;
};

} // namespace MyIDE::Editor
