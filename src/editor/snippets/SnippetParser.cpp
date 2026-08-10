#include "editor/snippets/SnippetParser.h"
#include <QRegularExpression>
#include <map>
#include <algorithm>

namespace MyIDE::Editor::Snippets {

ParsedSnippet SnippetParser::parse(const QString& templateBody) {
    ParsedSnippet result;
    QString resultText;
    std::map<int, SnippetPlaceholder> placeholderMap;

    int i = 0;
    int len = templateBody.length();

    while (i < len) {
        if (templateBody[i] == '$') {
            if (i + 1 < len && templateBody[i + 1] == '0') {
                result.finalCursorPos = resultText.length();
                i += 2;
                continue;
            } else if (i + 1 < len && templateBody[i + 1] == '{') {
                int endBrace = templateBody.indexOf('}', i + 2);
                if (endBrace != -1) {
                    QString token = templateBody.mid(i + 2, endBrace - (i + 2));
                    int colonIdx = token.indexOf(':');

                    int index = 0;
                    QString defaultText;

                    if (colonIdx != -1) {
                        index = token.left(colonIdx).toInt();
                        defaultText = token.mid(colonIdx + 1);
                    } else {
                        index = token.toInt();
                    }

                    int startPos = resultText.length();
                    resultText += defaultText;

                    PlaceholderOccurrence occ;
                    occ.startPos = startPos;
                    occ.length = defaultText.length();

                    auto& ph = placeholderMap[index];
                    ph.index = index;
                    if (ph.defaultText.isEmpty()) {
                        ph.defaultText = defaultText;
                    }
                    ph.occurrences.push_back(occ);

                    i = endBrace + 1;
                    continue;
                }
            } else {
                // Short form e.g. $1
                static const QRegularExpression numRegex(R"(^\$(\d+))");
                auto match = numRegex.match(templateBody.mid(i));
                if (match.hasMatch()) {
                    int index = match.captured(1).toInt();
                    if (index == 0) {
                        result.finalCursorPos = resultText.length();
                    } else {
                        auto& ph = placeholderMap[index];
                        ph.index = index;
                        PlaceholderOccurrence occ;
                        occ.startPos = resultText.length();
                        occ.length = ph.defaultText.length();
                        resultText += ph.defaultText;
                        ph.occurrences.push_back(occ);
                    }
                    i += match.capturedLength();
                    continue;
                }
            }
        }

        resultText += templateBody[i];
        i++;
    }

    result.expandedText = resultText;
    if (result.finalCursorPos == -1) {
        result.finalCursorPos = resultText.length();
    }

    for (const auto& [idx, ph] : placeholderMap) {
        result.placeholders.push_back(ph);
    }

    std::sort(result.placeholders.begin(), result.placeholders.end(), [](const SnippetPlaceholder& a, const SnippetPlaceholder& b) {
        return a.index < b.index;
    });

    return result;
}

} // namespace MyIDE::Editor::Snippets
