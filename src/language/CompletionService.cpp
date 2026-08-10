#include "language/CompletionService.h"
#include "language/LspClient.h"
#include "core/Logger.h"
#include <QRegularExpression>
#include <set>

namespace MyIDE::Language {

CompletionService::CompletionService(LspClient* lspClient, QObject* parent)
    : QObject(parent), m_lspClient(lspClient) {
}

void CompletionService::cancelPendingRequests() {
    m_latestRequestId++;
}

void CompletionService::requestCompletion(const CompletionRequestParams& params, std::function<void(const std::vector<Editor::CompletionItemData>&, int requestId)> callback) {
    m_latestRequestId++;
    int currentId = m_latestRequestId;

    if (!m_lspClient) {
        callback(localWordFallback("", params.linePrefix), currentId);
        return;
    }

    m_lspClient->requestCompletion(params.path, params.line, params.column, [this, currentId, params, callback](const std::vector<Editor::CompletionItemData>& items) {
        // Stale request protection
        if (currentId != m_latestRequestId) {
            MyIDE::Core::Logger::instance().debug("CompletionService", QString("Discarding stale completion request #%1").arg(currentId));
            return;
        }

        if (items.empty()) {
            auto fallbackItems = localWordFallback("", params.linePrefix);
            MyIDE::Core::Logger::instance().debug("CompletionService", QString("LSP returned 0 items. Local word fallback produced %1 items").arg(fallbackItems.size()));
            callback(fallbackItems, currentId);
        } else {
            MyIDE::Core::Logger::instance().info("CompletionService", QString("Completion request #%1 produced %2 items").arg(currentId).arg(items.size()));
            callback(items, currentId);
        }
    });
}

std::vector<Editor::CompletionItemData> CompletionService::localWordFallback(const QString& documentText, const QString& prefix) {
    std::vector<Editor::CompletionItemData> results;
    if (prefix.trimmed().isEmpty()) return results;

    static const QRegularExpression wordRegex(R"(\b[a-zA-Z_][a-zA-Z0-9_]*\b)");
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(documentText);

    std::set<QString> uniqueWords;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured(0);
        if (word.startsWith(prefix, Qt::CaseInsensitive) && word != prefix) {
            uniqueWords.insert(word);
        }
    }

    for (const auto& word : uniqueWords) {
        Editor::CompletionItemData item;
        item.label = word;
        item.detail = "local word";
        item.insertText = word;
        item.kind = 1; // Text
        results.push_back(item);
    }

    return results;
}

} // namespace MyIDE::Language
