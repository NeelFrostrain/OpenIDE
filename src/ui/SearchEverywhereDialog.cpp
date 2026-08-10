#include "SearchEverywhereDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QFileInfo>
#include <QApplication>
#include <QScreen>

SearchEverywhereDialog::SearchEverywhereDialog(SymbolIndex* index, QWidget* parent)
    : QDialog(parent), m_index(index) {
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    resize(700, 420);

    setStyleSheet("QDialog { background-color: #1e1f22; border: 1px solid #4e5157; border-radius: 6px; }"
                  "QLineEdit { background-color: #2b2d30; color: #dfe1e5; font-size: 14px; padding: 8px; border: 1px solid #4e5157; border-radius: 4px; }"
                  "QListWidget { background-color: #1e1f22; color: #dfe1e5; border: none; font-size: 13px; }"
                  "QListWidget::item { padding: 6px; border-bottom: 1px solid #2b2d30; }"
                  "QListWidget::item:selected { background-color: #2e436e; color: #ffffff; }");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search Everywhere (Files, Symbols, Actions)..."));
    mainLayout->addWidget(m_searchEdit);

    m_resultsList = new QListWidget(this);
    mainLayout->addWidget(m_resultsList);

    connect(m_searchEdit, &QLineEdit::textChanged, this, &SearchEverywhereDialog::onSearchTextChanged);
    connect(m_resultsList, &QListWidget::itemActivated, this, &SearchEverywhereDialog::onItemActivated);

    // Initial populate
    updateResults();
}

void SearchEverywhereDialog::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        reject();
        return;
    } else if (event->key() == Qt::Key_Down) {
        int cur = m_resultsList->currentRow();
        if (cur < m_resultsList->count() - 1) {
            m_resultsList->setCurrentRow(cur + 1);
        }
        return;
    } else if (event->key() == Qt::Key_Up) {
        int cur = m_resultsList->currentRow();
        if (cur > 0) {
            m_resultsList->setCurrentRow(cur - 1);
        }
        return;
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (m_resultsList->currentItem()) {
            onItemActivated(m_resultsList->currentItem());
            return;
        }
    }
    QDialog::keyPressEvent(event);
}

void SearchEverywhereDialog::onSearchTextChanged(const QString& text) {
    Q_UNUSED(text);
    updateResults();
}

void SearchEverywhereDialog::updateResults() {
    m_resultsList->clear();
    if (!m_index) return;

    QString query = m_searchEdit->text().trimmed();
    QList<SymbolItem> items = m_index->searchSymbols(query, 40);

    for (const auto& item : items) {
        auto* listItem = new QListWidgetItem(m_resultsList);
        QString prefix = QString("[%1] ").arg(item.kind.toUpper());
        QString displayText = prefix + item.name;

        if (!item.filePath.isEmpty()) {
            displayText += QString("   — %1:%2").arg(QFileInfo(item.filePath).fileName()).arg(item.line + 1);
        }

        listItem->setText(displayText);
        listItem->setData(Qt::UserRole, QVariant::fromValue(item.filePath));
        listItem->setData(Qt::UserRole + 1, item.line);
        listItem->setData(Qt::UserRole + 2, item.character);
    }

    if (m_resultsList->count() > 0) {
        m_resultsList->setCurrentRow(0);
    }
}

void SearchEverywhereDialog::onItemActivated(QListWidgetItem* item) {
    if (!item) return;

    SymbolItem symbol;
    symbol.filePath = item->data(Qt::UserRole).toString();
    symbol.line = item->data(Qt::UserRole + 1).toInt();
    symbol.character = item->data(Qt::UserRole + 2).toInt();

    emit symbolSelected(symbol);
    accept();
}
