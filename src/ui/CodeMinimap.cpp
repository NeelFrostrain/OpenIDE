#include "CodeMinimap.h"
#include <QPainter>
#include <QMouseEvent>
#include <QTextBlock>
#include <QScrollBar>

CodeMinimap::CodeMinimap(QPlainTextEdit* editor, QWidget* parent)
    : QWidget(parent ? parent : editor), m_editor(editor) {
    setFixedWidth(110);
    setStyleSheet("background-color: #1a1b1e; border-left: 1px solid #2b2d30;");

    if (m_editor) {
        connect(m_editor->verticalScrollBar(), &QScrollBar::valueChanged, this, [this] { update(); });
        connect(m_editor->document(), &QTextDocument::contentsChanged, this, [this] { update(); });
    }
}

void CodeMinimap::updateMinimap() {
    update();
}

void CodeMinimap::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    if (!m_editor) return;

    QPainter painter(this);
    painter.fillRect(rect(), QColor("#18191c"));

    QTextDocument* doc = m_editor->document();
    if (!doc) return;

    int totalBlocks = doc->blockCount();
    if (totalBlocks == 0) return;

    double scaleY = static_cast<double>(height()) / qMax(1, totalBlocks * 14);

    // Draw miniature text lines
    painter.setPen(QColor("#4e5157"));
    int blockIdx = 0;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next(), ++blockIdx) {
        int y = static_cast<int>(blockIdx * 14 * scaleY);
        QString txt = b.text().trimmed();
        if (!txt.isEmpty()) {
            int lineLen = qMin(static_cast<int>(txt.length() * 1.5), width() - 8);
            painter.drawLine(4, y, 4 + lineLen, y);
        }
    }

    // Highlight visible viewport window
    int visibleBlocks = qMax(1, m_editor->viewport()->height() / qMax(1, m_editor->fontMetrics().height()));
    int firstVisibleBlock = m_editor->verticalScrollBar()->value();
    int topY = static_cast<int>(firstVisibleBlock * 14 * scaleY);
    int hY = qMax(16, static_cast<int>(visibleBlocks * 14 * scaleY));

    painter.fillRect(QRect(0, topY, width(), hY), QColor(255, 255, 255, 25));
    painter.setPen(QColor("#4a9eff"));
    painter.drawRect(QRect(0, topY, width() - 1, hY - 1));
}

void CodeMinimap::mousePressEvent(QMouseEvent* event) {
    if (!m_editor) return;
    double scaleY = static_cast<double>(height()) / qMax(1, m_editor->document()->blockCount() * 14);
    int targetBlock = static_cast<int>(event->y() / scaleY);
    m_editor->verticalScrollBar()->setValue(targetBlock);
}

void CodeMinimap::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        mousePressEvent(event);
    }
}
