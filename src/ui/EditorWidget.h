#pragma once

#include <QPlainTextEdit>
#include <QWidget>

class EditorWidget;

// Small helper widget drawn to the left of the editor; forwards paint events
// back to the owning EditorWidget which knows the actual line numbers.
class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(EditorWidget* editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    EditorWidget* m_editor;
};

// A single open-file editor tab: plain text buffer + a line-number gutter and
// current-line highlight. Syntax highlighting (tree-sitter) and LSP diagnostics
// squiggles are added on top of this in a later milestone — kept deliberately
// minimal here so the prototype compiles with zero extra dependencies.
#include "lsp/LspTypes.h"

class TreeSitterHighlighter;
class CompletionPopup;

class EditorWidget : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit EditorWidget(QWidget* parent = nullptr);

    void loadFile(const QString& path);
    QString filePath() const { return m_filePath; }
    bool isModified() const { return document()->isModified(); }

    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth() const;

    void setDiagnostics(const QList<LspDiagnostic>& diagnostics);
    void gotoPosition(int line, int character);
    CompletionPopup* completionPopup() const { return m_completionPopup; }

signals:
    void completionRequested(const QString& path, int line, int character);
    void hoverRequested(const QString& path, int line, int character);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLineAndBracket();
    void updateLineNumberArea(const QRect& rect, int dy);

private:
    LineNumberArea* m_lineNumberArea;
    QString m_filePath;
    TreeSitterHighlighter* m_highlighter{nullptr};
    CompletionPopup* m_completionPopup{nullptr};
    QList<LspDiagnostic> m_diagnostics;
};
