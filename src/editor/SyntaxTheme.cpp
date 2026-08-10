#include "SyntaxTheme.h"
#include <cstring>

SyntaxTheme SyntaxTheme::createDarkTheme() {
    SyntaxTheme theme;

    // Keywords (#C678DD - Vibrant Purple)
    QTextCharFormat keywordFmt;
    keywordFmt.setForeground(QColor("#C678DD"));
    keywordFmt.setFontWeight(QFont::Bold);
    theme.formats[SyntaxCategory::Keyword] = keywordFmt;

    // Custom Types / Classes (#4EC9B0 - Vibrant Teal/Cyan)
    QTextCharFormat typeFmt;
    typeFmt.setForeground(QColor("#4EC9B0"));
    typeFmt.setFontWeight(QFont::Bold);
    theme.formats[SyntaxCategory::Type] = typeFmt;

    // Primitive Types (#569CD6 - Bright Soft Blue)
    QTextCharFormat primFmt;
    primFmt.setForeground(QColor("#569CD6"));
    primFmt.setFontWeight(QFont::Bold);
    theme.formats[SyntaxCategory::PrimitiveType] = primFmt;

    // Strings (#98C379 - Bright Emerald Green)
    QTextCharFormat strFmt;
    strFmt.setForeground(QColor("#98C379"));
    theme.formats[SyntaxCategory::String] = strFmt;

    // Comments (#5C6370 - Cool Slate Gray)
    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#5C6370"));
    commentFmt.setFontItalic(true);
    theme.formats[SyntaxCategory::Comment] = commentFmt;

    // Numbers (#D19A66 - Warm Amber Orange)
    QTextCharFormat numFmt;
    numFmt.setForeground(QColor("#D19A66"));
    theme.formats[SyntaxCategory::Number] = numFmt;

    // Functions & Methods (#61AFEF - Vibrant Sky Blue)
    QTextCharFormat funcFmt;
    funcFmt.setForeground(QColor("#61AFEF"));
    funcFmt.setFontWeight(QFont::Medium);
    theme.formats[SyntaxCategory::Function] = funcFmt;

    // Preprocessor Directives (#E06C75 - Vibrant Coral Red)
    QTextCharFormat prepFmt;
    prepFmt.setForeground(QColor("#E06C75"));
    prepFmt.setFontWeight(QFont::Bold);
    theme.formats[SyntaxCategory::Preprocessor] = prepFmt;

    // Operators & Punctuation (#C8D0E0 - Soft Silver)
    QTextCharFormat opFmt;
    opFmt.setForeground(QColor("#C8D0E0"));
    theme.formats[SyntaxCategory::Operator] = opFmt;

    // Variables & Identifiers (#ABB2BF - Soft White)
    QTextCharFormat varFmt;
    varFmt.setForeground(QColor("#ABB2BF"));
    theme.formats[SyntaxCategory::Variable] = varFmt;

    return theme;
}

QTextCharFormat SyntaxTheme::formatForNodeType(const char* nodeType) const {
    if (!nodeType) return {};

    SyntaxCategory cat = SyntaxCategory::Unknown;

    if (std::strcmp(nodeType, "primitive_type") == 0) {
        cat = SyntaxCategory::PrimitiveType;
    } else if (std::strcmp(nodeType, "type_identifier") == 0 ||
               std::strcmp(nodeType, "namespace_identifier") == 0) {
        cat = SyntaxCategory::Type;
    } else if (std::strcmp(nodeType, "comment") == 0) {
        cat = SyntaxCategory::Comment;
    } else if (std::strcmp(nodeType, "string_literal") == 0 ||
               std::strcmp(nodeType, "char_literal") == 0 ||
               std::strcmp(nodeType, "raw_string_literal") == 0 ||
               std::strcmp(nodeType, "system_lib_string") == 0) {
        cat = SyntaxCategory::String;
    } else if (std::strcmp(nodeType, "number_literal") == 0) {
        cat = SyntaxCategory::Number;
    } else if (std::strcmp(nodeType, "field_identifier") == 0) {
        cat = SyntaxCategory::Function;
    } else if (std::strncmp(nodeType, "preproc_", 8) == 0 ||
               std::strcmp(nodeType, "#include") == 0 ||
               std::strcmp(nodeType, "#define") == 0 ||
               std::strcmp(nodeType, "#ifdef") == 0 ||
               std::strcmp(nodeType, "#ifndef") == 0 ||
               std::strcmp(nodeType, "#endif") == 0 ||
               std::strcmp(nodeType, "#pragma") == 0) {
        cat = SyntaxCategory::Preprocessor;
    } else if (std::strcmp(nodeType, "=") == 0 || std::strcmp(nodeType, "+") == 0 ||
               std::strcmp(nodeType, "-") == 0 || std::strcmp(nodeType, "*") == 0 ||
               std::strcmp(nodeType, "/") == 0 || std::strcmp(nodeType, "&") == 0 ||
               std::strcmp(nodeType, "|") == 0 || std::strcmp(nodeType, "->") == 0 ||
               std::strcmp(nodeType, ".") == 0 || std::strcmp(nodeType, "::") == 0 ||
               std::strcmp(nodeType, "==") == 0 || std::strcmp(nodeType, "!=") == 0 ||
               std::strcmp(nodeType, "<=") == 0 || std::strcmp(nodeType, ">=") == 0 ||
               std::strcmp(nodeType, "&&") == 0 || std::strcmp(nodeType, "||") == 0) {
        cat = SyntaxCategory::Operator;
    } else if (std::strcmp(nodeType, "if") == 0 || std::strcmp(nodeType, "else") == 0 ||
               std::strcmp(nodeType, "while") == 0 || std::strcmp(nodeType, "for") == 0 ||
               std::strcmp(nodeType, "return") == 0 || std::strcmp(nodeType, "class") == 0 ||
               std::strcmp(nodeType, "struct") == 0 || std::strcmp(nodeType, "enum") == 0 ||
               std::strcmp(nodeType, "namespace") == 0 || std::strcmp(nodeType, "template") == 0 ||
               std::strcmp(nodeType, "typename") == 0 || std::strcmp(nodeType, "public") == 0 ||
               std::strcmp(nodeType, "private") == 0 || std::strcmp(nodeType, "protected") == 0 ||
               std::strcmp(nodeType, "virtual") == 0 || std::strcmp(nodeType, "override") == 0 ||
               std::strcmp(nodeType, "constexpr") == 0 || std::strcmp(nodeType, "const") == 0 ||
               std::strcmp(nodeType, "static") == 0 || std::strcmp(nodeType, "using") == 0 ||
               std::strcmp(nodeType, "typedef") == 0 || std::strcmp(nodeType, "auto") == 0 ||
               std::strcmp(nodeType, "explicit") == 0 || std::strcmp(nodeType, "noexcept") == 0 ||
               std::strcmp(nodeType, "new") == 0 || std::strcmp(nodeType, "delete") == 0) {
        cat = SyntaxCategory::Keyword;
    } else if (std::strcmp(nodeType, "identifier") == 0) {
        cat = SyntaxCategory::Variable;
    }

    auto it = formats.find(cat);
    if (it != formats.end()) {
        return it->second;
    }
    return {};
}
