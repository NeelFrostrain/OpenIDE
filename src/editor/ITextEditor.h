#pragma once

#include <QString>
#include <filesystem>
#include <vector>

namespace OpenIDE::Editor {

struct Diagnostic {
    int startLine = 0;
    int startColumn = 0;
    int endLine = 0;
    int endColumn = 0;
    int severity = 1; // 1: Error, 2: Warning, 3: Info, 4: Hint
    QString message;
    QString source;
};

enum class CompletionSource {
    Clangd,
    Keyword,
    LocalSymbol,
    Snippet,
    ProjectIndex
};

struct CompletionItemData {
    QString label;
    QString detail;
    QString documentation;
    QString insertText;
    int kind = 0;
    bool isSnippet = false;
    CompletionSource source = CompletionSource::Clangd;
};

class ITextEditor {
public:
    virtual ~ITextEditor() = default;

    virtual std::filesystem::path filePath() const = 0;
    virtual QString text() const = 0;
    virtual void setText(const QString& text) = 0;

    virtual void goToLine(int line, int column = 0) = 0;
    virtual void insertText(const QString& text) = 0;
    virtual void replaceSelection(const QString& text) = 0;

    virtual void setDiagnostics(const std::vector<Diagnostic>& diagnostics) = 0;
    virtual void setCompletions(const std::vector<CompletionItemData>& completions) = 0;
};

} // namespace OpenIDE::Editor
