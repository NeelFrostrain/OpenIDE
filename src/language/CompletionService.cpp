#include "language/CompletionService.h"
#include "language/LspClient.h"
#include "language/CompletionContext.h"
#include "language/CompletionRanking.h"
#include "core/Logger.h"
#include <QRegularExpression>
#include <QElapsedTimer>
#include <set>

namespace OpenIDE::Language {

CompletionService::CompletionService(LspClient* lspClient, QObject* parent)
    : QObject(parent), m_lspClient(lspClient) {
}

void CompletionService::cancelPendingRequests() {
    m_latestRequestId++;
}

void CompletionService::requestCompletion(const CompletionRequestParams& params, std::function<void(const std::vector<Editor::CompletionItemData>&, int requestId)> callback) {
    m_latestRequestId++;
    int currentId = m_latestRequestId;

    auto startTime = std::make_shared<QElapsedTimer>();
    startTime->start();

    CompletionContext ctx = ContextAnalyzer::analyze(params.linePrefix, params.triggerKind == CompletionTriggerKind::Invoked);

    if (!m_lspClient) {
        auto fallback = localWordFallback(params.documentText, ctx.typedPrefix);
        auto ranked = CompletionRanking::rankAndFilter(fallback, ctx);
        callback(ranked, currentId);
        return;
    }

    m_lspClient->requestCompletion(params.path, params.line, params.column, [this, currentId, params, ctx, startTime, callback](const std::vector<Editor::CompletionItemData>& items) {
        // Stale request protection
        if (currentId != m_latestRequestId) {
            OpenIDE::Core::Logger::instance().debug("CompletionService", QString("Discarding stale completion request #%1").arg(currentId));
            return;
        }

        qint64 lspMs = startTime->elapsed();

        std::vector<Editor::CompletionItemData> candidates = items;
        
        // Merge local document tokens so local variables/declarations are always present
        auto localWords = localWordFallback(params.documentText, ctx.typedPrefix);
        candidates.insert(candidates.end(), localWords.begin(), localWords.end());

        auto ranked = CompletionRanking::rankAndFilter(candidates, ctx);
        qint64 totalMs = startTime->elapsed();

        OpenIDE::Core::Logger::instance().info("Completion", QString("[Completion] TOTAL: %1 ms (LSP: %2 ms, candidates: %3, ranked: %4, prefix: '%5')")
            .arg(totalMs).arg(lspMs).arg(candidates.size()).arg(ranked.size()).arg(ctx.typedPrefix));

        callback(ranked, currentId);
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

} // namespace OpenIDE::Language
