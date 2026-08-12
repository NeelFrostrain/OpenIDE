#include "editor/snippets/SnippetSession.h"
#include "core/Logger.h"

namespace OpenIDE::Editor::Snippets {

SnippetSession::SnippetSession(QPlainTextEdit* editor, const ParsedSnippet& parsed, int insertPos)
    : m_editor(editor), m_parsed(parsed), m_insertPos(insertPos), m_active(true) {

    if (!m_editor) return;

    // Expand snippet text at insertPos within single undo block
    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.setPosition(m_insertPos);
    cursor.insertText(m_parsed.expandedText);
    cursor.endEditBlock();

    OpenIDE::Core::Logger::instance().info("SnippetSession", QString("[SnippetSession] Inserted expanded snippet length=%1").arg(m_parsed.expandedText.length()));

    if (!m_parsed.placeholders.empty()) {
        m_currentPlaceholderIdx = 0;
        selectCurrentPlaceholder();
    } else {
        cursor.setPosition(m_insertPos + m_parsed.finalCursorPos);
        m_editor->setTextCursor(cursor);
        m_active = false;
    }
}

void SnippetSession::cancel() {
    m_active = false;
}

void SnippetSession::selectCurrentPlaceholder() {
    if (!m_active || !m_editor || m_currentPlaceholderIdx < 0 || m_currentPlaceholderIdx >= static_cast<int>(m_parsed.placeholders.size())) {
        return;
    }

    const auto& ph = m_parsed.placeholders[m_currentPlaceholderIdx];
    if (ph.occurrences.empty()) return;

    int selStart = m_insertPos + ph.occurrences[0].startPos;
    int selEnd = selStart + ph.occurrences[0].length;

    QTextCursor cursor = m_editor->textCursor();
    cursor.setPosition(selStart);
    cursor.setPosition(selEnd, QTextCursor::KeepAnchor);
    m_editor->setTextCursor(cursor);
}

bool SnippetSession::nextPlaceholder() {
    if (!m_active) return false;

    if (m_currentPlaceholderIdx + 1 < static_cast<int>(m_parsed.placeholders.size())) {
        m_currentPlaceholderIdx++;
        selectCurrentPlaceholder();
        return true;
    } else {
        // Move cursor to final $0 position and end snippet session
        QTextCursor cursor = m_editor->textCursor();
        cursor.setPosition(m_insertPos + m_parsed.finalCursorPos);
        m_editor->setTextCursor(cursor);
        m_active = false;
        OpenIDE::Core::Logger::instance().info("SnippetSession", "[SnippetSession] Reached $0. Snippet session finished.");
        return true;
    }
}

bool SnippetSession::previousPlaceholder() {
    if (!m_active) return false;

    if (m_currentPlaceholderIdx > 0) {
        m_currentPlaceholderIdx--;
        selectCurrentPlaceholder();
        return true;
    }
    return false;
}

void SnippetSession::updateMirroredPlaceholders() {
    // Optional placeholder mirror synchronization
}

} // namespace OpenIDE::Editor::Snippets
