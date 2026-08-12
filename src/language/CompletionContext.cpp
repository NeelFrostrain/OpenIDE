#include "language/CompletionContext.h"
#include <QRegularExpression>

namespace OpenIDE::Language {

CompletionContext ContextAnalyzer::analyze(const QString& lineBeforeCursor, bool isManual) {
    CompletionContext ctx;
    ctx.fullLineBeforeCursor = lineBeforeCursor;
    ctx.isManualTrigger = isManual;

    QString trimmed = lineBeforeCursor.trimmed();

    // Check for comment
    if (trimmed.startsWith("//") || trimmed.startsWith("/*")) {
        ctx.kind = ContextKind::Comment;
        return ctx;
    }

    // Check for #include directive
    if (lineBeforeCursor.contains("#include")) {
        int quotePos = lineBeforeCursor.lastIndexOf('"');
        int anglePos = lineBeforeCursor.lastIndexOf('<');

        if (quotePos != -1 && (anglePos == -1 || quotePos > anglePos)) {
            ctx.kind = ContextKind::IncludePath;
            ctx.typedPrefix = lineBeforeCursor.mid(quotePos + 1);
            return ctx;
        } else if (anglePos != -1) {
            ctx.kind = ContextKind::IncludeSystem;
            ctx.typedPrefix = lineBeforeCursor.mid(anglePos + 1);
            return ctx;
        }
    }

    // Check for string literal (odd number of quotes)
    int quoteCount = lineBeforeCursor.count('"');
    if (quoteCount % 2 != 0) {
        ctx.kind = ContextKind::StringLiteral;
        return ctx;
    }

    // Extract typed token under cursor
    static const QRegularExpression tokenRegex(R"([a-zA-Z_][a-zA-Z0-9_]*$)");
    QRegularExpressionMatch match = tokenRegex.match(lineBeforeCursor);
    if (match.hasMatch()) {
        ctx.typedPrefix = match.captured(0);
    }

    // Check member access (-> or .)
    QString beforeToken = lineBeforeCursor.left(lineBeforeCursor.length() - ctx.typedPrefix.length()).trimmed();
    if (beforeToken.endsWith("->") || beforeToken.endsWith(".")) {
        ctx.kind = ContextKind::MemberAccess;
    } else if (beforeToken.endsWith("::")) {
        ctx.kind = ContextKind::ScopeResolution;
    } else {
        ctx.kind = ContextKind::GeneralCode;
    }

    return ctx;
}

} // namespace OpenIDE::Language
