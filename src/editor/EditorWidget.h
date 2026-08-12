#pragma once

#include "editor/ITextEditor.h"
#include "editor/Document.h"
#include "syntax/TreeSitterHighlighter.h"
#include "editor/CompletionPopup.h"
#include <QPlainTextEdit>
#include <QWidget>
#include <set>

namespace OpenIDE::Editor {

class EditorWidget : public QPlainTextEdit, public ITextEditor {
    Q_OBJECT

public:
    explicit EditorWidget(Document* doc, QWidget* parent = nullptr);
    ~EditorWidget() override = default;

    Document* documentModel() const { return m_document; }

    // ITextEditor interface
    std::filesystem::path filePath() const override;
    QString text() const override;
    void setText(const QString& text) override;

    void goToLine(int line, int column = 0) override;
    void insertText(const QString& text) override;
    void replaceSelection(const QString& text) override;

    void setDiagnostics(const std::vector<Diagnostic>& diagnostics) override;
    void setCompletions(const std::vector<CompletionItemData>& completions) override;

    void setCompletionController(class CompletionController* controller) { m_completionController = controller; }

    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();

signals:
    void completionRequested(const QString& prefix, int line, int column);
    void breakpointToggled(int line, bool added);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect& rect, int dy);
    void onTextChanged();

private:
    Document* m_document = nullptr;
    QWidget* m_lineNumberArea = nullptr;
    Syntax::TreeSitterHighlighter* m_highlighter = nullptr;
    CompletionPopup* m_completionPopup = nullptr;
    class CompletionController* m_completionController = nullptr;

    std::set<int> m_breakpoints;
    std::vector<Diagnostic> m_diagnostics;
};

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(EditorWidget* editor) : QWidget(editor), m_editor(editor) {}

    QSize sizeHint() const override {
        return QSize(m_editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        m_editor->lineNumberAreaPaintEvent(event);
    }

private:
    EditorWidget* m_editor;
};

} // namespace OpenIDE::Editor
