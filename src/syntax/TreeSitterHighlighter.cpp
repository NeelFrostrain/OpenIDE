#include "syntax/TreeSitterHighlighter.h"
#include "ui/ThemeManager.h"
#include "core/Logger.h"
#include <QColor>
#include <QFont>
#include <cstring>
#include <unordered_set>

namespace MyIDE::Syntax {

static const std::unordered_set<std::string> s_keywords = {
    "class", "struct", "enum", "namespace", "template", "typename", "using",
    "public", "private", "protected", "virtual", "override", "const", "constexpr",
    "static", "inline", "extern", "mutable", "volatile", "auto", "decltype",
    "if", "else", "for", "while", "do", "switch", "case", "default", "return",
    "break", "continue", "new", "delete", "sizeof", "alignof", "noexcept", "throw",
    "try", "catch", "this", "nullptr", "true", "false"
};

static const std::unordered_set<std::string> s_builtinTypes = {
    "void", "int", "float", "double", "bool", "char", "short", "long",
    "unsigned", "signed", "size_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "int8_t", "int16_t", "int32_t", "int64_t"
};

static const std::unordered_set<std::string> s_unrealMacros = {
    "UCLASS", "USTRUCT", "UENUM", "UFUNCTION", "UPROPERTY", "UPARAM", "UMETA", "UINTERFACE",
    "GENERATED_BODY", "GENERATED_UCLASS_BODY", "IMPLEMENT_PRIMARY_GAME_MODULE",
    "DECLARE_DYNAMIC_MULTICAST_DELEGATE", "DECLARE_DYNAMIC_DELEGATE", "DECLARE_MULTICAST_DELEGATE"
};

TreeSitterHighlighter::TreeSitterHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    m_parser = ts_parser_new();
    if (m_parser) {
        ts_parser_set_language(m_parser, tree_sitter_cpp());
    }
}

TreeSitterHighlighter::~TreeSitterHighlighter() {
    if (m_tree) {
        ts_tree_delete(m_tree);
    }
    if (m_parser) {
        ts_parser_delete(m_parser);
    }
}

void TreeSitterHighlighter::parseFullDocument() {
    if (!document() || !m_parser) return;

    std::string currentText = document()->toPlainText().toStdString();
    if (currentText == m_cachedSource && m_tree) {
        return;
    }

    m_cachedSource = currentText;
    if (m_tree) {
        ts_tree_delete(m_tree);
    }

    m_tree = ts_parser_parse_string(m_parser, nullptr, m_cachedSource.c_str(), static_cast<uint32_t>(m_cachedSource.length()));
}

void TreeSitterHighlighter::highlightBlock(const QString& text) {
    Q_UNUSED(text);
    if (!m_parser) return;

    parseFullDocument();
    if (!m_tree) return;

    uint32_t blockStart = static_cast<uint32_t>(currentBlock().position());
    uint32_t blockEnd = blockStart + static_cast<uint32_t>(currentBlock().length());

    TSNode rootNode = ts_tree_root_node(m_tree);
    traverseAndHighlight(rootNode, blockStart, blockEnd);
}

void TreeSitterHighlighter::traverseAndHighlight(TSNode node, uint32_t blockStart, uint32_t blockEnd) {
    uint32_t nodeStart = ts_node_start_byte(node);
    uint32_t nodeEnd = ts_node_end_byte(node);

    // Overlap check
    if (nodeEnd <= blockStart || nodeStart >= blockEnd) {
        return;
    }

    QTextCharFormat fmt = determineFormat(node);
    if (fmt.isValid()) {
        uint32_t highlightStart = std::max(nodeStart, blockStart);
        uint32_t highlightEnd = std::min(nodeEnd, blockEnd);

        if (highlightEnd > highlightStart) {
            int relStart = static_cast<int>(highlightStart - blockStart);
            int len = static_cast<int>(highlightEnd - highlightStart);
            setFormat(relStart, len, fmt);
        }
    }

    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        traverseAndHighlight(child, blockStart, blockEnd);
    }
}

QTextCharFormat TreeSitterHighlighter::determineFormat(TSNode node) const {
    const char* typeStr = ts_node_type(node);
    if (!typeStr) return {};

    std::string type(typeStr);
    const auto& colors = UI::ThemeManager::instance().colors();

    // 1. Comments (Muted Green Italic)
    if (type == "comment") {
        QTextCharFormat fmt;
        fmt.setForeground(colors.comment);
        fmt.setFontItalic(true);
        return fmt;
    }

    // 2. Strings & Character Literals (Orange/Brown #CE9178)
    if (type == "string_literal" || type == "char_literal" || type == "system_lib_string" || type == "raw_string_literal") {
        QTextCharFormat fmt;
        fmt.setForeground(colors.string);
        return fmt;
    }

    // 3. Numbers (Light Green #B5CEA8)
    if (type == "number_literal") {
        QTextCharFormat fmt;
        fmt.setForeground(colors.number);
        return fmt;
    }

    // 4. Preprocessor Directive Keywords (#include, #define, #pragma, #ifdef, #endif)
    if (type == "#include" || type == "#define" || type == "#if" || type == "#ifdef" || type == "#ifndef" || type == "#else" || type == "#elif" || type == "#endif" || type == "#pragma") {
        QTextCharFormat fmt;
        fmt.setForeground(colors.macro);
        fmt.setFontWeight(QFont::Bold);
        return fmt;
    }

    // 5. Built-in Types & User Types/Classes (Teal/Cyan #4EC9B0)
    if (type == "primitive_type" || type == "type_identifier" || type == "struct_specifier" || type == "class_specifier" || type == "enum_specifier") {
        QTextCharFormat fmt;
        fmt.setForeground(colors.type);
        return fmt;
    }

    // 6. Keywords (Purple/Magenta #C586C0 Bold)
    if (s_keywords.count(type)) {
        QTextCharFormat fmt;
        fmt.setForeground(colors.keyword);
        fmt.setFontWeight(QFont::Bold);
        return fmt;
    }

    // 7. Built-in Types (Teal #4EC9B0)
    if (s_builtinTypes.count(type)) {
        QTextCharFormat fmt;
        fmt.setForeground(colors.type);
        return fmt;
    }

    // Node content checking for text tokens
    uint32_t startByte = ts_node_start_byte(node);
    uint32_t endByte = ts_node_end_byte(node);
    if (endByte > startByte && endByte <= m_cachedSource.length()) {
        std::string tokenText = m_cachedSource.substr(startByte, endByte - startByte);

        // Keywords check by text token
        if (s_keywords.count(tokenText)) {
            QTextCharFormat fmt;
            fmt.setForeground(colors.keyword);
            fmt.setFontWeight(QFont::Bold);
            return fmt;
        }

        // Built-in types check by text token
        if (s_builtinTypes.count(tokenText)) {
            QTextCharFormat fmt;
            fmt.setForeground(colors.type);
            return fmt;
        }

        // Unreal Macros check (UCLASS, UPROPERTY, UFUNCTION, GENERATED_BODY)
        if (s_unrealMacros.count(tokenText) || tokenText.rfind("UCLASS", 0) == 0 || tokenText.rfind("UPROPERTY", 0) == 0 || tokenText.rfind("UFUNCTION", 0) == 0 || tokenText.rfind("GENERATED_BODY", 0) == 0) {
            QTextCharFormat fmt;
            fmt.setForeground(colors.macro);
            fmt.setFontWeight(QFont::Bold);
            return fmt;
        }
    }

    // 8. Functions & Methods (Yellow/Gold #DCDCAA)
    if (type == "identifier" || type == "field_identifier") {
        TSNode parent = ts_node_parent(node);
        if (!ts_node_is_null(parent)) {
            const char* parentType = ts_node_type(parent);
            if (parentType) {
                std::string pType(parentType);
                if (pType == "function_declarator" || pType == "call_expression" || pType == "function_definition") {
                    QTextCharFormat fmt;
                    fmt.setForeground(colors.function);
                    return fmt;
                } else if (pType == "field_declaration" || pType == "parameter_declaration") {
                    QTextCharFormat fmt;
                    fmt.setForeground(colors.parameter);
                    return fmt;
                } else if (pType == "qualified_identifier" || pType == "namespace_definition" || pType == "using_declaration") {
                    QTextCharFormat fmt;
                    fmt.setForeground(colors.nameSpace);
                    return fmt;
                }
            }
        }
    }

    return {};
}

} // namespace MyIDE::Syntax
