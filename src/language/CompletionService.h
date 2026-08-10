#pragma once

#include "editor/ITextEditor.h"
#include <QObject>
#include <filesystem>
#include <vector>
#include <functional>
#include <memory>

namespace MyIDE::Language {

class LspClient;

enum class CompletionTriggerKind {
    Invoked,          // Ctrl+Space
    TriggerCharacter, // ->, ., ::, #include "
    TriggerForIncompleteCompletions
};

struct CompletionRequestParams {
    std::filesystem::path path;
    int line = 1;
    int column = 0;
    QString linePrefix;
    QString documentText;
    CompletionTriggerKind triggerKind = CompletionTriggerKind::Invoked;
};

class CompletionService : public QObject {
    Q_OBJECT

public:
    explicit CompletionService(LspClient* lspClient, QObject* parent = nullptr);

    void requestCompletion(const CompletionRequestParams& params, std::function<void(const std::vector<Editor::CompletionItemData>&, int requestId)> callback);
    void cancelPendingRequests();

    std::vector<Editor::CompletionItemData> localWordFallback(const QString& documentText, const QString& prefix);

private:
    LspClient* m_lspClient = nullptr;
    int m_latestRequestId = 0;
};

} // namespace MyIDE::Language
