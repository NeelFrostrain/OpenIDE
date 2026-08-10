#pragma once

#include <QString>
#include <QList>

struct LspPosition {
    int line{0};
    int character{0};
};

struct LspRange {
    LspPosition start;
    LspPosition end;
};

enum class LspDiagnosticSeverity {
    Error = 1,
    Warning = 2,
    Information = 3,
    Hint = 4
};

struct LspDiagnostic {
    LspRange range;
    LspDiagnosticSeverity severity{LspDiagnosticSeverity::Error};
    QString code;
    QString source;
    QString message;
};

struct LspCompletionItem {
    QString label;
    int kind{0};
    QString detail;
    QString documentation;
    QString insertText;
};

struct LspLocation {
    QString uri;
    LspRange range;
};
