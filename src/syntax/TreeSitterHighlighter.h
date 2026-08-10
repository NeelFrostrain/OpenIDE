#pragma once

#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextCharFormat>
#include <tree_sitter/api.h>
#include <memory>
#include <unordered_map>
#include <string>

extern "C" {
const TSLanguage* tree_sitter_cpp(void);
}

namespace MyIDE::Syntax {

class TreeSitterHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit TreeSitterHighlighter(QTextDocument* parent = nullptr);
    ~TreeSitterHighlighter() override;

protected:
    void highlightBlock(const QString& text) override;

private:
    void parseFullDocument();
    void traverseAndHighlight(TSNode node, uint32_t blockStart, uint32_t blockEnd);
    QTextCharFormat determineFormat(TSNode node) const;

    TSParser* m_parser = nullptr;
    TSTree* m_tree = nullptr;
    std::string m_cachedSource;
};

} // namespace MyIDE::Syntax
