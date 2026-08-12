#pragma once

#include "editor/snippets/SnippetDefinition.h"
#include "editor/ITextEditor.h"
#include <vector>
#include <map>

namespace OpenIDE::Editor::Snippets {

class SnippetRegistry {
public:
    static SnippetRegistry& instance();

    void registerSnippet(const SnippetDefinition& snippet);
    std::vector<SnippetDefinition> findMatching(const QString& prefix, const QString& language = "cpp") const;
    const SnippetDefinition* findByTrigger(const QString& trigger, const QString& language = "cpp") const;

    std::vector<CompletionItemData> toCompletionItems(const QString& prefix, const QString& language = "cpp") const;

private:
    SnippetRegistry();
    void registerDefaultCppSnippets();
    void registerUnrealSnippets();

    std::vector<SnippetDefinition> m_snippets;
};

} // namespace OpenIDE::Editor::Snippets
