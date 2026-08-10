#include "editor/DocumentManager.h"
#include "core/Logger.h"
#include <QFile>
#include <QTextStream>

namespace MyIDE::Editor {

DocumentManager& DocumentManager::instance() {
    static DocumentManager s_instance;
    return s_instance;
}

Document* DocumentManager::openDocument(const std::filesystem::path& path) {
    std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(path);
    if (auto* existing = getDocumentByPath(canonicalPath)) {
        return existing;
    }

    QFile file(QString::fromStdString(canonicalPath.string()));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        MyIDE::Core::Logger::instance().error("DocumentManager", QString("Could not open file: %1").arg(file.errorString()));
        return nullptr;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    DocumentId id = m_nextId++;
    auto doc = std::make_unique<Document>(id, canonicalPath, content);
    Document* docPtr = doc.get();
    m_documents[id] = std::move(doc);

    emit documentOpened(docPtr);
    MyIDE::Core::Logger::instance().info("DocumentManager", QString("Opened document [%1]: %2").arg(id).arg(docPtr->fileName()));
    return docPtr;
}

Document* DocumentManager::createUntitledDocument() {
    DocumentId id = m_nextId++;
    std::filesystem::path path = QString("Untitled-%1.cpp").arg(id).toStdString();
    auto doc = std::make_unique<Document>(id, path, "");
    Document* docPtr = doc.get();
    m_documents[id] = std::move(doc);

    emit documentOpened(docPtr);
    return docPtr;
}

bool DocumentManager::closeDocument(DocumentId id) {
    auto it = m_documents.find(id);
    if (it != m_documents.end()) {
        m_documents.erase(it);
        emit documentClosed(id);
        return true;
    }
    return false;
}

Document* DocumentManager::getDocument(DocumentId id) const {
    auto it = m_documents.find(id);
    if (it != m_documents.end()) {
        return it->second.get();
    }
    return nullptr;
}

Document* DocumentManager::getDocumentByPath(const std::filesystem::path& path) const {
    std::filesystem::path target = std::filesystem::weakly_canonical(path);
    for (const auto& [id, doc] : m_documents) {
        if (std::filesystem::equivalent(doc->path(), target)) {
            return doc.get();
        }
    }
    return nullptr;
}

void DocumentManager::saveAll() {
    for (const auto& [id, doc] : m_documents) {
        if (doc->isDirty()) {
            doc->save();
        }
    }
}

std::vector<Document*> DocumentManager::openDocuments() const {
    std::vector<Document*> result;
    result.reserve(m_documents.size());
    for (const auto& [id, doc] : m_documents) {
        result.push_back(doc.get());
    }
    return result;
}

} // namespace MyIDE::Editor
