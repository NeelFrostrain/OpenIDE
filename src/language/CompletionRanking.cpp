#include "language/CompletionRanking.h"
#include "language/IncludeIndex.h"
#include "editor/snippets/SnippetRegistry.h"
#include "core/Logger.h"
#include <algorithm>
#include <unordered_map>

namespace OpenIDE::Language {

std::vector<Editor::CompletionItemData> CompletionRanking::cppKeywords() {
    static const std::vector<Editor::CompletionItemData> items = []() {
        static const std::vector<QString> keywords = {
            "class", "struct", "enum", "namespace", "template", "typename", "using",
            "public", "private", "protected", "virtual", "override", "const", "constexpr",
            "static", "inline", "extern", "mutable", "volatile", "auto", "decltype",
            "if", "else", "for", "while", "do", "switch", "case", "default", "return",
            "break", "continue", "new", "delete", "sizeof", "alignof", "noexcept", "throw",
            "try", "catch", "this", "nullptr", "true", "false", "void", "int", "float",
            "double", "bool", "char", "short", "long", "unsigned", "signed", "int32_t", "uint32_t"
        };

        std::vector<Editor::CompletionItemData> list;
        for (const auto& kw : keywords) {
            Editor::CompletionItemData item;
            item.label = kw;
            item.detail = "keyword";
            item.insertText = kw;
            item.kind = 14;
            item.source = Editor::CompletionSource::Keyword;
            list.push_back(item);
        }
        return list;
    }();

    return items;
}

int CompletionRanking::calculateScore(const Editor::CompletionItemData& item, const CompletionContext& ctx) {
    const QString& prefix = ctx.typedPrefix;
    const QString& label = item.label;

    int score = 100;

    if (prefix.isEmpty()) {
        if (item.kind == 14) score += 200; // Keywords
        return score;
    }

    // 1. Exact Token Match (+4000 for snippet, +3000 for symbol, +2000 for keyword)
    if (label == prefix) {
        if (item.isSnippet) {
            score += 4000;
        } else if (item.kind == 14) {
            score += 2000;
        } else {
            score += 3000;
        }
    }
    // 2. Case-Sensitive Prefix Match (+2500 for snippet, +1500 for normal)
    else if (label.startsWith(prefix, Qt::CaseSensitive)) {
        if (item.isSnippet) {
            score += 2500;
        } else {
            score += 1500;
        }
    }
    // 3. Case-Insensitive Prefix Match (+1000)
    else if (label.startsWith(prefix, Qt::CaseInsensitive)) {
        score += 1000;
    }
    // 4. Non-Prefix Substring Match (-2000 heavy penalty)
    else if (label.contains(prefix, Qt::CaseInsensitive)) {
        score -= 2000;
    }
    // 5. Unrelated / Weak Fuzzy (-3000)
    else {
        score -= 3000;
    }

    // Context & Kind Scoring
    if (ctx.kind == ContextKind::GeneralCode) {
        if (item.kind == 6 || item.kind == 13) score += 700; // Variable / Local Variable
        if (item.kind == 25) score += 700;                  // Parameter
        if (item.kind == 2 || item.kind == 3)  score += 600; // Function / Method
        if (item.kind == 7 || item.kind == 22) score += 600; // Class / Struct
        if (item.kind == 5 || item.kind == 10) score += 500; // Field / Property
        if (item.kind == 1)  score += 600;                  // Local document word
        if (item.isSnippet || item.kind == 15) score += 800;   // Code Snippets / Live Templates
        if (item.kind == 14) score += 500;                  // Keywords
    } else if (ctx.kind == ContextKind::MemberAccess) {
        if (item.kind == 2 || item.kind == 3)  score += 800; // Methods / Functions
        if (item.kind == 5 || item.kind == 10) score += 800; // Fields / Properties
        if (item.kind == 1)  score += 500;                  // Local document word
        if (item.kind == 14) score -= 1000;                 // Penalty for keywords during member access
    }

    // Heavy penalty for template parameters & qualified namespace traits in label (e.g. std::is_class<class Ty>)
    if (label.contains("<") || label.contains("::")) {
        score -= 600;
    }

    return score;
}

std::vector<Editor::CompletionItemData> CompletionRanking::rankAndFilter(
    const std::vector<Editor::CompletionItemData>& rawItems,
    const CompletionContext& ctx) {

    if ((ctx.kind == ContextKind::Comment || ctx.kind == ContextKind::StringLiteral) && !ctx.isManualTrigger) {
        return {};
    }

    std::vector<Editor::CompletionItemData> candidates;

    if (ctx.kind == ContextKind::IncludePath || ctx.kind == ContextKind::IncludeSystem) {
        bool isSystem = (ctx.kind == ContextKind::IncludeSystem);
        // Include context: Filter rawItems to header/file items only
        for (const auto& item : rawItems) {
            if (item.kind == 17 || item.source == Editor::CompletionSource::Clangd) {
                candidates.push_back(item);
            }
        }
        // Fallback: Fetch from cached IncludeIndex
        auto indexedHeaders = IncludeIndex::instance().getIncludeCompletions(ctx.typedPrefix, isSystem);
        candidates.insert(candidates.end(), indexedHeaders.begin(), indexedHeaders.end());
    } else {
        candidates = rawItems;
        if (ctx.kind == ContextKind::GeneralCode) {
            auto keywords = cppKeywords();
            candidates.insert(candidates.end(), keywords.begin(), keywords.end());

            auto snippets = Editor::Snippets::SnippetRegistry::instance().toCompletionItems(ctx.typedPrefix, "cpp");
            candidates.insert(candidates.end(), snippets.begin(), snippets.end());
        }
    }

    // Deduplication & Merging Stage (Preserving Snippet vs Keyword distinctions)
    std::unordered_map<std::string, Editor::CompletionItemData> mergedMap;

    for (const auto& item : candidates) {
        if (!ctx.typedPrefix.isEmpty()) {
            bool matches = item.label.startsWith(ctx.typedPrefix, Qt::CaseInsensitive);
            if (!matches && ctx.typedPrefix.length() >= 2 && item.label.contains(ctx.typedPrefix, Qt::CaseInsensitive)) {
                matches = true;
            }
            if (!matches) continue;
        }

        std::string typeTag = item.isSnippet ? "snippet" : (item.kind == 14 ? "keyword" : "symbol");
        std::string labelKey = item.label.toLower().toStdString() + "_" + typeTag;
        auto it = mergedMap.find(labelKey);
        if (it == mergedMap.end()) {
            mergedMap[labelKey] = item;
        } else {
            // Merge rules: Clangd > Snippet > ProjectIndex > Keyword > LocalSymbol
            if (item.source == Editor::CompletionSource::Clangd && it->second.source != Editor::CompletionSource::Clangd) {
                mergedMap[labelKey] = item;
            }
        }
    }

    // Score & Pair
    struct ScoredItem {
        Editor::CompletionItemData item;
        int score = 0;
    };

    std::vector<ScoredItem> scored;
    for (const auto& [key, item] : mergedMap) {
        int s = calculateScore(item, ctx);
        scored.push_back({item, s});
    }

    // Sort by Score descending
    std::stable_sort(scored.begin(), scored.end(), [](const ScoredItem& a, const ScoredItem& b) {
        return a.score > b.score;
    });

    // Extract top items
    std::vector<Editor::CompletionItemData> result;
    for (size_t i = 0; i < scored.size() && i < 12; ++i) {
        result.push_back(scored[i].item);
    }

    if (!scored.empty()) {
        OpenIDE::Core::Logger::instance().info("CompletionRanking", QString("Ranked %1 candidates for prefix '%2' (Top score: %3 for '%4')")
            .arg(scored.size())
            .arg(ctx.typedPrefix)
            .arg(scored[0].score)
            .arg(scored[0].item.label));
    }

    return result;
}

} // namespace OpenIDE::Language
