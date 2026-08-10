#pragma once

#include <QString>

namespace MyIDE::Language {

enum class ContextKind {
    GeneralCode,
    MemberAccess,       // -> or .
    ScopeResolution,    // ::
    IncludePath,        // #include "..."
    StringLiteral,      // "..."
    Comment             // // or /* ... */
};

struct CompletionContext {
    ContextKind kind = ContextKind::GeneralCode;
    QString typedPrefix;
    QString fullLineBeforeCursor;
    bool isManualTrigger = false;
};

class ContextAnalyzer {
public:
    static CompletionContext analyze(const QString& lineBeforeCursor, bool isManual = false);
};

} // namespace MyIDE::Language
