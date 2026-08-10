#pragma once

#include <QTextCharFormat>
#include <QColor>
#include <QString>
#include <unordered_map>

enum class SyntaxCategory {
    Keyword,
    Type,
    PrimitiveType,
    String,
    Comment,
    Number,
    Function,
    Preprocessor,
    Operator,
    Punctuation,
    Variable,
    Unknown
};

struct SyntaxTheme {
    std::unordered_map<SyntaxCategory, QTextCharFormat> formats;

    static SyntaxTheme createDarkTheme();
    QTextCharFormat formatForNodeType(const char* nodeType) const;
};
