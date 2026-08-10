#pragma once

#include "editor/snippets/SnippetDefinition.h"
#include <QString>

namespace MyIDE::Editor::Snippets {

class SnippetParser {
public:
    static ParsedSnippet parse(const QString& templateBody);
};

} // namespace MyIDE::Editor::Snippets
