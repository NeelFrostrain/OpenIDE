#pragma once

#include "editor/snippets/SnippetDefinition.h"
#include <QPlainTextEdit>
#include <QTextCursor>
#include <vector>

namespace MyIDE::Editor::Snippets {

class SnippetSession {
public:
    SnippetSession(QPlainTextEdit* editor, const ParsedSnippet& parsed, int insertPos);

    bool isActive() const { return m_active; }
    void cancel();

    bool nextPlaceholder();
    bool previousPlaceholder();
    void updateMirroredPlaceholders();

private:
    void selectCurrentPlaceholder();

    QPlainTextEdit* m_editor = nullptr;
    ParsedSnippet m_parsed;
    int m_insertPos = 0;
    int m_currentPlaceholderIdx = -1;
    bool m_active = false;
};

} // namespace MyIDE::Editor::Snippets
