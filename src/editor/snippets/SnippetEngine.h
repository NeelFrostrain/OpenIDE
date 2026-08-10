#pragma once

#include "editor/snippets/SnippetDefinition.h"
#include "editor/snippets/SnippetRegistry.h"
#include "editor/snippets/SnippetSession.h"
#include <QObject>
#include <QPlainTextEdit>
#include <memory>

namespace MyIDE::Editor::Snippets {

class SnippetEngine : public QObject {
    Q_OBJECT

public:
    static SnippetEngine& instance();

    bool isSessionActive() const;
    bool expandSnippet(QPlainTextEdit* editor, const QString& trigger, const QString& language = "cpp");
    bool expandSnippetTemplate(QPlainTextEdit* editor, const QString& templateBody, int replaceLength);

    bool handleKeyEvent(QPlainTextEdit* editor, QKeyEvent* event);
    void cancelActiveSession();

private:
    SnippetEngine() = default;

    std::unique_ptr<SnippetSession> m_activeSession;
};

} // namespace MyIDE::Editor::Snippets
