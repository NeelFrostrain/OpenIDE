#pragma once

#include <QString>
#include <QStringList>
#include <vector>

namespace MyIDE::Editor::Snippets {

struct PlaceholderOccurrence {
    int startPos = 0; // Relative offset in expanded text
    int length = 0;
};

struct SnippetPlaceholder {
    int index = 0; // 1, 2, 3... (0 is final cursor)
    QString defaultText;
    std::vector<PlaceholderOccurrence> occurrences;
};

struct ParsedSnippet {
    QString expandedText;
    std::vector<SnippetPlaceholder> placeholders;
    int finalCursorPos = -1; // Offset of $0
};

struct SnippetDefinition {
    QString trigger;
    QStringList aliases;
    QString description;
    QString category;
    QString body;
    QString language = "cpp";
    int priority = 100;
};

} // namespace MyIDE::Editor::Snippets
