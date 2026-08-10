#pragma once

#include <QSyntaxHighlighter>
#include "SyntaxTheme.h"
#include <QByteArray>

typedef struct TSTree TSTree;
typedef struct TSParser TSParser;
typedef struct TSNode TSNode;

class TreeSitterHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit TreeSitterHighlighter(QTextDocument* parent = nullptr);
    ~TreeSitterHighlighter() override;

    void updateTree();

protected:
    void highlightBlock(const QString& text) override;

private:
    TSParser* m_parser = nullptr;
    TSTree* m_tree = nullptr;
    SyntaxTheme m_theme;
    QByteArray m_sourceBytes;
    bool m_treeNeedsUpdate{true};

    void highlightNode(TSNode node, int blockRow, int textLen, int& count);
};
