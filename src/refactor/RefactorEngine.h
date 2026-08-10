#pragma once

#include <QString>
#include <QStringList>

struct ExtractMethodResult {
    QString extractedFunctionCode;
    QString replacementCallText;
    bool success{false};
};

class RefactorEngine {
public:
    static ExtractMethodResult extractMethod(const QString& selectedText, const QString& functionName);
    static QString expandSnippet(const QString& abbreviation);
};
