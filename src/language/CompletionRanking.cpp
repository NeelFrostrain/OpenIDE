#include "language/CompletionRanking.h"
#include "core/Logger.h"
#include <algorithm>
#include <unordered_map>

namespace MyIDE::Language {

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

    // 1. Exact Token Match (+3000)
    if (label == prefix) {
        score += 3000;
    }
    // 2. Case-Sensitive Prefix Match (+1500)
    else if (label.startsWith(prefix, Qt::CaseSensitive)) {
        score += 1500;
    }
    // 3. Case-Insensitive Prefix Match (+1000)
    else if (label.startsWith(prefix, Qt::CaseInsensitive)) {
        score += 1000;
    }
    // 4. Substring / Qualified Substring Match (-800 penalty)
    else if (label.contains(prefix, Qt::CaseInsensitive)) {
        score += 100;
        // Heavy penalty if typed prefix occurs as part of another word (e.g. std::is_class containing "class")
        score -= 800;
    }
    // 5. Unrelated / Weak Fuzzy (-1000)
    else {
        score -= 1000;
    }

    // Context & Kind Scoring
    if (ctx.kind == ContextKind::GeneralCode) {
        if (item.kind == 6 || item.kind == 13) score += 700; // Variable / Local Variable
        if (item.kind == 25) score += 700;                  // Parameter
        if (item.kind == 2 || item.kind == 3)  score += 600; // Function / Method
        if (item.kind == 7 || item.kind == 22) score += 600; // Class / Struct
        if (item.kind == 5 || item.kind == 10) score += 500; // Field / Property
        if (item.kind == 1)  score += 600;                  // Local document word
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

    std::vector<Editor::CompletionItemData> candidates = rawItems;

    if (ctx.kind == ContextKind::GeneralCode) {
        auto keywords = cppKeywords();
        candidates.insert(candidates.end(), keywords.begin(), keywords.end());
    }

    // Deduplication & Merging Stage
    std::unordered_map<std::string, Editor::CompletionItemData> mergedMap;

    for (const auto& item : candidates) {
        if (!ctx.typedPrefix.isEmpty() && !item.label.contains(ctx.typedPrefix, Qt::CaseInsensitive)) {
            continue;
        }

        std::string labelKey = item.label.toLower().toStdString();
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
        MyIDE::Core::Logger::instance().info("CompletionRanking", QString("Ranked %1 candidates for prefix '%2' (Top score: %3 for '%4')")
            .arg(scored.size())
            .arg(ctx.typedPrefix)
            .arg(scored[0].score)
            .arg(scored[0].item.label));
    }

    return result;
}

} // namespace MyIDE::Language
