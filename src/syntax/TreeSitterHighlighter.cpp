#include "syntax/TreeSitterHighlighter.h"
#include "core/Logger.h"
#include <QColor>
#include <QFont>
#include <cstring>

namespace MyIDE::Syntax {

TreeSitterHighlighter::TreeSitterHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    m_parser = ts_parser_new();
    if (m_parser) {
        ts_parser_set_language(m_parser, tree_sitter_cpp());
    }

    // Color definitions
    QTextCharFormat keywordFmt;
    keywordFmt.setForeground(QColor("#569CD6"));
    keywordFmt.setFontWeight(QFont::Bold);

    QTextCharFormat typeFmt;
    typeFmt.setForeground(QColor("#4EC9B0"));

    QTextCharFormat funcFmt;
    funcFmt.setForeground(QColor("#DCDCAA"));

    QTextCharFormat stringFmt;
    stringFmt.setForeground(QColor("#CE9178"));

    QTextCharFormat numberFmt;
    numberFmt.setForeground(QColor("#B5CEA8"));

    QTextCharFormat commentFmt;
    commentFmt.setForeground(QColor("#6A9955"));
    commentFmt.setFontItalic(true);

    QTextCharFormat macroFmt;
    macroFmt.setForeground(QColor("#C586C0"));
    macroFmt.setFontWeight(QFont::Bold);

    // Cache rules
    m_formatCache["primitive_type"] = typeFmt;
    m_formatCache["type_identifier"] = typeFmt;
    m_formatCache["struct_specifier"] = typeFmt;
    m_formatCache["class_specifier"] = typeFmt;

    m_formatCache["function_declarator"] = funcFmt;
    m_formatCache["call_expression"] = funcFmt;
    m_formatCache["field_identifier"] = funcFmt;

    m_formatCache["string_literal"] = stringFmt;
    m_formatCache["char_literal"] = stringFmt;
    m_formatCache["number_literal"] = numberFmt;

    m_formatCache["comment"] = commentFmt;

    m_formatCache["preproc_include"] = macroFmt;
    m_formatCache["preproc_def"] = macroFmt;
    m_formatCache["preproc_function_def"] = macroFmt;
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
    if (!m_parser) return;

    parseFullDocument();
    if (!m_tree) return;

    int blockStartPos = currentBlock().position();
    int blockLength = text.length();

    TSNode rootNode = ts_tree_root_node(m_tree);
    
    // Traverse AST nodes overlapping this block range
    uint32_t startByte = static_cast<uint32_t>(blockStartPos);
    uint32_t endByte = static_cast<uint32_t>(blockStartPos + blockLength);

    TSNode descendant = ts_node_descendant_for_byte_range(rootNode, startByte, endByte);
    if (!ts_node_is_null(descendant)) {
        highlightNode(descendant, QString::fromStdString(m_cachedSource));
    }
}

void TreeSitterHighlighter::highlightNode(TSNode node, const QString& fullText) {
    uint32_t startByte = ts_node_start_byte(node);
    uint32_t endByte = ts_node_end_byte(node);
    int blockStart = currentBlock().position();
    int blockLen = currentBlock().length();

    const char* type = ts_node_type(node);
    QTextCharFormat fmt = formatForNodeType(type);

    if (fmt.isValid() && startByte >= static_cast<uint32_t>(blockStart) && endByte <= static_cast<uint32_t>(blockStart + blockLen)) {
        setFormat(startByte - blockStart, endByte - startByte, fmt);
    }

    uint32_t childCount = ts_node_child_count(node);
    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(node, i);
        highlightNode(child, fullText);
    }
}

QTextCharFormat TreeSitterHighlighter::formatForNodeType(const char* type) const {
    if (!type) return {};
    auto it = m_formatCache.find(type);
    if (it != m_formatCache.end()) {
        return it->second;
    }

    // Default fallback keywords checking
    static const std::unordered_map<std::string, QTextCharFormat> keywords = {
        {"class", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); f.setFontWeight(QFont::Bold); return f; }() },
        {"struct", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); f.setFontWeight(QFont::Bold); return f; }() },
        {"public", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); return f; }() },
        {"private", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); return f; }() },
        {"protected", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); return f; }() },
        {"override", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); return f; }() },
        {"virtual", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); return f; }() },
        {"void", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); return f; }() },
        {"int", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); return f; }() },
        {"float", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); return f; }() },
        {"double", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); return f; }() },
        {"bool", []{ QTextCharFormat f; f.setForeground(QColor("#569CD6")); return f; }() },
        {"return", []{ QTextCharFormat f; f.setForeground(QColor("#D8A0DF")); f.setFontWeight(QFont::Bold); return f; }() },
    };

    auto kwIt = keywords.find(type);
    if (kwIt != keywords.end()) {
        return kwIt->second;
    }

    return {};
}

} // namespace MyIDE::Syntax
