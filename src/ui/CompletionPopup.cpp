#include "CompletionPopup.h"
#include <QVBoxLayout>
#include <QKeyEvent>

CompletionPopup::CompletionPopup(QWidget* parent) : QListWidget(parent) {
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_ShowWithoutActivating);

    setStyleSheet("QListWidget { background-color: #1e1f22; border: 1px solid #4e5157; border-radius: 6px; font-size: 13px; font-family: 'Cascadia Code', monospace; padding: 4px; }"
                  "QListWidget::item { padding: 5px 8px; border-radius: 3px; color: #dfe1e5; }"
                  "QListWidget::item:selected { background-color: #2e436e; color: #ffffff; }");

    hide();
}

void CompletionPopup::setCompletionItems(const QList<LspCompletionItem>& items) {
    m_allItems = items;
    filter("");
}

void CompletionPopup::filter(const QString& prefix) {
    clear();
    for (const auto& item : m_allItems) {
        if (prefix.isEmpty() || item.label.startsWith(prefix, Qt::CaseInsensitive)) {
            auto* listItem = new QListWidgetItem(this);

            QString kindTag = "[SYM]";
            if (item.kind == 2 || item.kind == 3) kindTag = "[FUNC]";
            else if (item.kind == 5 || item.kind == 6) kindTag = "[VAR]";
            else if (item.kind == 7 || item.kind == 22) kindTag = "[CLASS]";
            else if (item.kind == 14) kindTag = "[KEYWORD]";

            listItem->setText(QString("%1  %2  %3").arg(item.label, kindTag, item.detail));
            listItem->setData(Qt::UserRole, item.insertText);
        }
    }

    if (count() > 0) {
        setCurrentRow(0);
        int h = qMin(8, count()) * 26 + 10;
        resize(360, h);
        show();
        raise();
    } else {
        hide();
    }
}

void CompletionPopup::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        hide();
        return;
    }
    QListWidget::keyPressEvent(event);
}
