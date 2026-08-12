#pragma once

#include "editor/ITextEditor.h"
#include "language/CompletionContext.h"
#include <vector>

namespace OpenIDE::Language {

class CompletionRanking {
public:
    static std::vector<Editor::CompletionItemData> rankAndFilter(
        const std::vector<Editor::CompletionItemData>& rawItems,
        const CompletionContext& ctx);

private:
    static int calculateScore(const Editor::CompletionItemData& item, const CompletionContext& ctx);
    static std::vector<Editor::CompletionItemData> cppKeywords();
};

} // namespace OpenIDE::Language
