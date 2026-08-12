#include "editor/snippets/SnippetEngine.h"
#include "editor/snippets/SnippetParser.h"
#include "core/Logger.h"
#include <QKeyEvent>

namespace OpenIDE::Editor::Snippets {

SnippetEngine& SnippetEngine::instance() {
    static SnippetEngine s_instance;
    return s_instance;
}

bool SnippetEngine::isSessionActive() const {
    return m_activeSession && m_activeSession->isActive();
}

void SnippetEngine::cancelActiveSession() {
    if (m_activeSession) {
        m_activeSession->cancel();
        m_activeSession.reset();
    }
}

bool SnippetEngine::expandSnippet(QPlainTextEdit* editor, const QString& trigger, const QString& language) {
    if (!editor) return false;

    const SnippetDefinition* snip = SnippetRegistry::instance().findByTrigger(trigger, language);
    if (!snip) return false;

    return expandSnippetTemplate(editor, snip->body, trigger.length());
}

bool SnippetEngine::expandSnippetTemplate(QPlainTextEdit* editor, const QString& templateBody, int replaceLength) {
    if (!editor) return false;

    cancelActiveSession();

    QTextCursor cursor = editor->textCursor();
    if (replaceLength > 0) {
        cursor.beginEditBlock();
        for (int i = 0; i < replaceLength; ++i) {
            cursor.deletePreviousChar();
        }
        cursor.endEditBlock();
    }

    int insertPos = cursor.position();
    ParsedSnippet parsed = SnippetParser::parse(templateBody);

    m_activeSession = std::make_unique<SnippetSession>(editor, parsed, insertPos);
    return true;
}

bool SnippetEngine::handleKeyEvent(QPlainTextEdit* editor, QKeyEvent* event) {
    Q_UNUSED(editor);
    if (!isSessionActive()) return false;

    if (event->key() == Qt::Key_Tab) {
        if (event->modifiers() & Qt::ShiftModifier) {
            return m_activeSession->previousPlaceholder();
        } else {
            return m_activeSession->nextPlaceholder();
        }
    } else if (event->key() == Qt::Key_Escape) {
        cancelActiveSession();
        return true;
    }

    return false;
}

} // namespace OpenIDE::Editor::Snippets
