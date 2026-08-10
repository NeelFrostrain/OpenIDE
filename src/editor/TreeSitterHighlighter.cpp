#include "TreeSitterHighlighter.h"
extern "C" {
#include <tree_sitter/api.h>
const TSLanguage *tree_sitter_cpp(void);
}
#include <QTextBlock>
#include <QDebug>
#include <cstring>

TreeSitterHighlighter::TreeSitterHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent), m_theme(SyntaxTheme::createDarkTheme()) {
    m_parser = ts_parser_new();
    ts_parser_set_language(m_parser, tree_sitter_cpp());

    if (parent) {
        connect(parent, &QTextDocument::contentsChanged, this, [this] {
            m_treeNeedsUpdate = true;
        });
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

void TreeSitterHighlighter::updateTree() {
    if (!document()) return;

    QString text = document()->toPlainText();
    m_sourceBytes = text.toUtf8();

    TSTree* oldTree = m_tree;
    m_tree = ts_parser_parse_string(m_parser, oldTree, m_sourceBytes.constData(), m_sourceBytes.size());

    if (oldTree) {
        ts_tree_delete(oldTree);
    }

    m_treeNeedsUpdate = false;
}

void TreeSitterHighlighter::highlightBlock(const QString& text) {
    if (text.isEmpty()) return;

    if (m_treeNeedsUpdate || !m_tree) {
        updateTree();
    }

    if (!m_tree) return;

    int blockRow = currentBlock().blockNumber();
    TSNode root = ts_tree_root_node(m_tree);

    if (ts_node_is_null(root)) return;

    int count = 0;
    highlightNode(root, blockRow, text.length(), count);
}

void TreeSitterHighlighter::highlightNode(TSNode node, int blockRow, int textLen, int& count) {
    if (ts_node_is_null(node)) return;

    TSPoint start = ts_node_start_point(node);
    TSPoint end = ts_node_end_point(node);

    if ((int)start.row > blockRow || (int)end.row < blockRow) {
        return;
    }

    const char* type = ts_node_type(node);
    if (!type) return;

    uint32_t childCount = ts_node_child_count(node);

    // Atomic tokens or leaf nodes
    bool isAtomicToken = (childCount == 0) ||
                         (std::strcmp(type, "comment") == 0) ||
                         (std::strcmp(type, "string_literal") == 0) ||
                         (std::strcmp(type, "raw_string_literal") == 0) ||
                         (std::strcmp(type, "char_literal") == 0) ||
                         (std::strcmp(type, "system_lib_string") == 0);

    if (isAtomicToken) {
        QTextCharFormat format = m_theme.formatForNodeType(type);
        if (!format.isEmpty()) {
            int colStart = (start.row == (uint32_t)blockRow) ? (int)start.column : 0;
            int colEnd = (end.row == (uint32_t)blockRow) ? (int)end.column : textLen;
            int length = colEnd - colStart;

            if (length > 0 && colStart >= 0 && colStart < textLen) {
                setFormat(colStart, qMin(length, textLen - colStart), format);
                count++;
            }
        }
    } else {
        for (uint32_t i = 0; i < childCount; ++i) {
            TSNode child = ts_node_child(node, i);
            if (!ts_node_is_null(child)) {
                highlightNode(child, blockRow, textLen, count);
            }
        }
    }
}
