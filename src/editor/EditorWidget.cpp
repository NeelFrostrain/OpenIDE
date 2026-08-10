#include "editor/EditorWidget.h"
#include "editor/CompletionController.h"
#include "ui/ThemeManager.h"
#include "core/Logger.h"
#include <QPainter>
#include <QTextBlock>
#include <QKeyEvent>

namespace MyIDE::Editor {

EditorWidget::EditorWidget(Document* doc, QWidget* parent)
    : QPlainTextEdit(parent), m_document(doc) {
    
    m_lineNumberArea = new LineNumberArea(this);

    setFont(UI::ThemeManager::instance().editorFont());
    setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));
    setLineWrapMode(QPlainTextEdit::NoWrap);

    // Dark editor palette styling with subtle selection background (preserving syntax colors)
    QPalette p = palette();
    p.setColor(QPalette::Highlight, QColor("#214D73"));
    setPalette(p);

    setStyleSheet(R"(
        QPlainTextEdit {
            background-color: #1E1E1E;
            color: #D4D4D4;
            border: none;
            selection-background-color: #214D73;
        }
    )");

    if (m_document) {
        setPlainText(m_document->content());
        connect(this, &QPlainTextEdit::textChanged, this, &EditorWidget::onTextChanged);
    }

    m_highlighter = new Syntax::TreeSitterHighlighter(document());
    m_completionPopup = new CompletionPopup(this);

    connect(m_completionPopup, &CompletionPopup::itemSelected, [this](const CompletionItemData& item) {
        QTextCursor cursor = textCursor();
        cursor.select(QTextCursor::WordUnderCursor);
        QString replacement = item.insertText.isEmpty() ? item.label : item.insertText;

        // If simple identifier without parens, append () for functions
        if ((item.kind == 2 || item.kind == 3) && !replacement.contains("(")) {
            replacement += "()";
        }

        cursor.insertText(replacement);
    });

    connect(this, &QPlainTextEdit::blockCountChanged, this, &EditorWidget::updateLineNumberAreaWidth);
    connect(this, &QPlainTextEdit::updateRequest, this, &EditorWidget::updateLineNumberArea);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this, &EditorWidget::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

std::filesystem::path EditorWidget::filePath() const {
    return m_document ? m_document->path() : std::filesystem::path();
}

QString EditorWidget::text() const {
    return toPlainText();
}

void EditorWidget::setText(const QString& text) {
    setPlainText(text);
}

void EditorWidget::goToLine(int line, int column) {
    QTextBlock block = document()->findBlockByLineNumber(line - 1);
    if (block.isValid()) {
        QTextCursor cursor(block);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);
        setTextCursor(cursor);
        centerCursor();
    }
}

void EditorWidget::insertText(const QString& text) {
    textCursor().insertText(text);
}

void EditorWidget::replaceSelection(const QString& text) {
    textCursor().insertText(text);
}

void EditorWidget::setDiagnostics(const std::vector<Diagnostic>& diagnostics) {
    m_diagnostics = diagnostics;
    viewport()->update();
}

void EditorWidget::setCompletions(const std::vector<CompletionItemData>& completions) {
    if (completions.empty()) {
        m_completionPopup->hide();
        return;
    }
    m_completionPopup->setCompletions(completions);
    QRect cRect = cursorRect();
    QPoint globalPos = viewport()->mapToGlobal(cRect.bottomLeft());
    m_completionPopup->move(globalPos);
    m_completionPopup->show();
}

int EditorWidget::lineNumberAreaWidth() {
    int digits = 1;
    int maxBlock = qMax(1, blockCount());
    while (maxBlock >= 10) {
        maxBlock /= 10;
        ++digits;
    }
    int space = 25 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void EditorWidget::updateLineNumberAreaWidth(int /* newBlockCount */) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void EditorWidget::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void EditorWidget::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void EditorWidget::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor("#282828");
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void EditorWidget::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor("#1E1E1E"));

    // Separator line
    painter.setPen(QColor("#333333"));
    painter.drawLine(m_lineNumberArea->width() - 1, event->rect().top(), m_lineNumberArea->width() - 1, event->rect().bottom());

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            int lineNumber = blockNumber + 1;
            
            // Draw Breakpoint Indicator
            if (m_breakpoints.count(lineNumber)) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor("#E51400"));
                painter.drawEllipse(4, top + 3, 10, 10);
            }

            // Draw Line Number
            bool isCurrent = (blockNumber == textCursor().blockNumber());
            painter.setPen(isCurrent ? QColor("#C6C6C6") : QColor("#858585"));
            painter.drawText(0, top, m_lineNumberArea->width() - 8, fontMetrics().height(),
                             Qt::AlignRight, QString::number(lineNumber));
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void EditorWidget::mousePressEvent(QMouseEvent* event) {
    if (event->position().x() < lineNumberAreaWidth()) {
        QTextBlock block = firstVisibleBlock();
        int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
        int bottom = top + qRound(blockBoundingRect(block).height());

        while (block.isValid()) {
            if (event->position().y() >= top && event->position().y() <= bottom) {
                int line = block.blockNumber() + 1;
                bool exists = m_breakpoints.count(line) > 0;
                if (exists) m_breakpoints.erase(line);
                else m_breakpoints.insert(line);

                emit breakpointToggled(line, !exists);
                m_lineNumberArea->update();
                return;
            }
            block = block.next();
            top = bottom;
            bottom = top + qRound(blockBoundingRect(block).height());
        }
    }
    QPlainTextEdit::mousePressEvent(event);
}

void EditorWidget::focusOutEvent(QFocusEvent* event) {
    if (m_completionPopup && m_completionPopup->isVisible()) {
        m_completionPopup->hide();
    }
    QPlainTextEdit::focusOutEvent(event);
}

void EditorWidget::wheelEvent(QWheelEvent* event) {
    if (event->modifiers() & Qt::ControlModifier) {
        int delta = event->angleDelta().y();
        if (delta > 0) {
            zoomIn(1);
        } else if (delta < 0) {
            zoomOut(1);
        }
        setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));
        updateLineNumberAreaWidth(0);
        event->accept();
        return;
    }
    QPlainTextEdit::wheelEvent(event);
}

void EditorWidget::keyPressEvent(QKeyEvent* event) {
    // If completion controller is active, let it handle navigation and acceptance keys first
    if (m_completionController && m_completionController->handleKeyPress(event)) {
        event->accept();
        return;
    }

    // Ctrl + Plus / Ctrl + Minus / Ctrl + 0 Zooming
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->key() == Qt::Key_Plus || event->key() == Qt::Key_Equal) {
            zoomIn(1);
            setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));
            updateLineNumberAreaWidth(0);
            event->accept();
            return;
        } else if (event->key() == Qt::Key_Minus) {
            zoomOut(1);
            setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));
            updateLineNumberAreaWidth(0);
            event->accept();
            return;
        } else if (event->key() == Qt::Key_0) {
            setFont(UI::ThemeManager::instance().editorFont());
            setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));
            updateLineNumberAreaWidth(0);
            event->accept();
            return;
        }
    }

    // Ctrl + Space manual completion trigger
    if ((event->modifiers() & Qt::ControlModifier) && event->key() == Qt::Key_Space) {
        QTextCursor cursor = textCursor();
        int line = cursor.blockNumber() + 1;
        int col = cursor.positionInBlock();
        QString lineText = cursor.block().text().left(col);
        MyIDE::Core::Logger::instance().info("EditorWidget", QString("Ctrl+Space triggered manual completion at line=%1 col=%2 prefix='%3'").arg(line).arg(col).arg(lineText));
        emit completionRequested(lineText, line, col);
        event->accept();
        return;
    }

    QPlainTextEdit::keyPressEvent(event);

    if (event->text() == "->" || event->text() == "." || event->text() == ":" || event->text() == "#" || (event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z)) {
        QTextCursor cursor = textCursor();
        int line = cursor.blockNumber() + 1;
        int col = cursor.positionInBlock();
        QString lineText = cursor.block().text().left(col);
        emit completionRequested(lineText, line, col);
    }
}

void EditorWidget::onTextChanged() {
    if (m_document) {
        m_document->setContent(toPlainText());
    }
}

} // namespace MyIDE::Editor
