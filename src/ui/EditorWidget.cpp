#include "EditorWidget.h"
#include "editor/TreeSitterHighlighter.h"
#include "CompletionPopup.h"

#include <QFile>
#include <QTextStream>
#include <QPainter>
#include <QTextBlock>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QToolTip>

LineNumberArea::LineNumberArea(EditorWidget* editor)
    : QWidget(editor), m_editor(editor) {}

QSize LineNumberArea::sizeHint() const {
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent* event) {
    m_editor->lineNumberAreaPaintEvent(event);
}

EditorWidget::EditorWidget(QWidget* parent) : QPlainTextEdit(parent) {
    m_lineNumberArea = new LineNumberArea(this);
    m_highlighter = new TreeSitterHighlighter(document());
    m_completionPopup = new CompletionPopup(this);

    connect(m_completionPopup, &CompletionPopup::itemSelected, this, [this](const QString& text) {
        QTextCursor c = textCursor();
        c.insertText(text);
        setTextCursor(c);
    });

    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mono.setPointSize(11);
    setFont(mono);

    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabStopDistance(4 * fontMetrics().horizontalAdvance(' '));

    connect(this, &EditorWidget::blockCountChanged, this, &EditorWidget::updateLineNumberAreaWidth);
    connect(this, &EditorWidget::updateRequest, this, &EditorWidget::updateLineNumberArea);
    connect(this, &EditorWidget::cursorPositionChanged, this, &EditorWidget::highlightCurrentLineAndBracket);

    updateLineNumberAreaWidth(0);
    highlightCurrentLineAndBracket();
}

void EditorWidget::loadFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    QTextStream in(&file);
    setPlainText(in.readAll());
    document()->setModified(false);
    m_filePath = path;
}

int EditorWidget::lineNumberAreaWidth() const {
    int digits = 1;
    int maxLines = qMax(1, blockCount());
    while (maxLines >= 10) {
        maxLines /= 10;
        ++digits;
    }
    return 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void EditorWidget::updateLineNumberAreaWidth(int) {
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void EditorWidget::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy != 0) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void EditorWidget::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void EditorWidget::gotoPosition(int line, int character) {
    QTextBlock block = document()->findBlockByNumber(line);
    if (block.isValid()) {
        QTextCursor cursor(block);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, character);
        setTextCursor(cursor);
        setFocus();
    }
}

void EditorWidget::setDiagnostics(const QList<LspDiagnostic>& diagnostics) {
    m_diagnostics = diagnostics;
    highlightCurrentLineAndBracket();
}

void EditorWidget::highlightCurrentLineAndBracket() {
    QList<QTextEdit::ExtraSelection> extraSelections;

    // Current line highlight
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor("#26282b");
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    // Bracket matching
    QTextCursor tc = textCursor();
    QString docText = document()->toPlainText();
    int pos = tc.position();

    if (pos >= 0 && pos < docText.length()) {
        QChar ch = docText.at(pos);
        QChar matchCh;
        int dir = 0;

        if (ch == '(') { matchCh = ')'; dir = 1; }
        else if (ch == ')') { matchCh = '('; dir = -1; }
        else if (ch == '{') { matchCh = '}'; dir = 1; }
        else if (ch == '}') { matchCh = '{'; dir = -1; }
        else if (ch == '[') { matchCh = ']'; dir = 1; }
        else if (ch == ']') { matchCh = '['; dir = -1; }

        if (dir != 0) {
            int depth = 1;
            int cur = pos + dir;
            while (cur >= 0 && cur < docText.length()) {
                if (docText.at(cur) == ch) depth++;
                else if (docText.at(cur) == matchCh) depth--;

                if (depth == 0) {
                    // Match found!
                    QTextEdit::ExtraSelection sel1, sel2;
                    sel1.format.setBackground(QColor("#3b514d"));
                    sel1.format.setFontWeight(QFont::Bold);
                    sel1.cursor = tc;
                    sel1.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

                    sel2.format.setBackground(QColor("#3b514d"));
                    sel2.format.setFontWeight(QFont::Bold);
                    sel2.cursor = QTextCursor(document());
                    sel2.cursor.setPosition(cur);
                    sel2.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

                    extraSelections.append(sel1);
                    extraSelections.append(sel2);
                    break;
                }
                cur += dir;
            }
        }
    }

    // Diagnostic squiggles
    for (const auto& d : m_diagnostics) {
        QTextBlock b = document()->findBlockByNumber(d.range.start.line);
        if (b.isValid()) {
            QTextEdit::ExtraSelection sel;
            sel.cursor = QTextCursor(b);
            sel.cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, d.range.start.character);

            int startPos = sel.cursor.position();
            QTextBlock endB = document()->findBlockByNumber(d.range.end.line);
            int endPos = endB.isValid() ? endB.position() + d.range.end.character : startPos + 1;
            if (endPos <= startPos) endPos = startPos + 1;

            sel.cursor.setPosition(endPos, QTextCursor::KeepAnchor);

            sel.format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
            sel.format.setUnderlineColor((d.severity == LspDiagnosticSeverity::Error) ? QColor("#e53935") : QColor("#e6c07b"));
            extraSelections.append(sel);
        }
    }

    setExtraSelections(extraSelections);
}

void EditorWidget::keyPressEvent(QKeyEvent* event) {
    if (m_completionPopup && m_completionPopup->isVisible()) {
        if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up ||
            event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return || event->key() == Qt::Key_Tab) {
            m_completionPopup->keyPressEvent(event);
            return;
        } else if (event->key() == Qt::Key_Escape) {
            m_completionPopup->hide();
            return;
        }
    }

    // Auto-closing pairs
    if (event->text() == "(") {
        QPlainTextEdit::keyPressEvent(event);
        insertPlainText(")");
        moveCursor(QTextCursor::Left);
        return;
    } else if (event->text() == "{") {
        QPlainTextEdit::keyPressEvent(event);
        insertPlainText("}");
        moveCursor(QTextCursor::Left);
        return;
    } else if (event->text() == "[") {
        QPlainTextEdit::keyPressEvent(event);
        insertPlainText("]");
        moveCursor(QTextCursor::Left);
        return;
    } else if (event->text() == "\"") {
        QPlainTextEdit::keyPressEvent(event);
        insertPlainText("\"");
        moveCursor(QTextCursor::Left);
        return;
    }

    // Smart auto-indent on Enter
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QTextCursor c = textCursor();
        QString currentLine = c.block().text().left(c.positionInBlock());
        int indent = 0;
        while (indent < currentLine.length() && currentLine.at(indent).isSpace()) {
            indent++;
        }
        QString indentStr = currentLine.left(indent);
        if (currentLine.trimmed().endsWith("{")) {
            indentStr += "    ";
        }

        QPlainTextEdit::keyPressEvent(event);
        insertPlainText(indentStr);
        return;
    }

    QPlainTextEdit::keyPressEvent(event);

    // Trigger completion request
    QTextCursor c = textCursor();
    int line = c.blockNumber();
    int col = c.positionInBlock();

    if (event->text() == "." || event->text() == ">" || event->text() == ":" ||
        (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Space)) {

        QRect r = cursorRect();
        r.moveTopLeft(viewport()->mapToGlobal(r.topLeft()));
        r.moveTop(r.top() + fontMetrics().height());
        m_completionPopup->move(r.topLeft());

        emit completionRequested(m_filePath, line, col);
    }
}

void EditorWidget::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor("#1e1f22"));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor("#5c5f63"));

            // Check if line has diagnostic warning/error
            for (const auto& d : m_diagnostics) {
                if (d.range.start.line == blockNumber) {
                    painter.setPen((d.severity == LspDiagnosticSeverity::Error) ? QColor("#e53935") : QColor("#e6c07b"));
                    break;
                }
            }

            painter.drawText(0, top, m_lineNumberArea->width() - 8, fontMetrics().height(),
                              Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}
