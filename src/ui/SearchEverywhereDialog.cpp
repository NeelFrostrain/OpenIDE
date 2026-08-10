#include "ui/SearchEverywhereDialog.h"
#include <QKeyEvent>
#include <QHeaderView>

namespace MyIDE::UI {

SearchEverywhereDialog::SearchEverywhereDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setFixedSize(650, 420);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(8);

    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText("Search everywhere (Files, Classes, Symbols, Actions)...");
    m_searchInput->setFont(QFont("Segoe UI", 11));

    m_resultsList = new QListWidget(this);
    m_resultsList->setFont(QFont("Consolas", 10));

    setStyleSheet(R"(
        QDialog {
            background-color: #252526;
            border: 1px solid #3E3E42;
            border-radius: 6px;
        }
        QLineEdit {
            background-color: #1E1E1E;
            color: #FFFFFF;
            border: 1px solid #007ACC;
            border-radius: 4px;
            padding: 8px 12px;
        }
        QListWidget {
            background-color: #1E1E1E;
            color: #CCCCCC;
            border: 1px solid #333333;
            border-radius: 4px;
        }
        QListWidget::item {
            padding: 6px 10px;
        }
        QListWidget::item:selected {
            background-color: #04395E;
            color: #FFFFFF;
        }
        QListWidget::item:hover {
            background-color: #2A2D2E;
        }
    )");

    mainLayout->addWidget(m_searchInput);
    mainLayout->addWidget(m_resultsList);

    connect(m_searchInput, &QLineEdit::textChanged, this, &SearchEverywhereDialog::onSearchTextChanged);
    connect(m_resultsList, &QListWidget::itemActivated, this, &SearchEverywhereDialog::onItemActivated);
}

void SearchEverywhereDialog::setFiles(const std::vector<std::filesystem::path>& files) {
    m_files = files;
    onSearchTextChanged("");
}

void SearchEverywhereDialog::onSearchTextChanged(const QString& text) {
    m_resultsList->clear();
    m_currentResults.clear();

    for (const auto& file : m_files) {
        QString fileName = QString::fromStdString(file.filename().string());
        QString filePath = QString::fromStdString(file.string());

        if (text.isEmpty() || fileName.contains(text, Qt::CaseInsensitive) || filePath.contains(text, Qt::CaseInsensitive)) {
            SearchResultItem item;
            item.title = fileName;
            item.subtitle = filePath;
            item.category = "File";
            item.path = file;

            m_currentResults.push_back(item);

            QString displayStr = QString("📄  %1   —   %2").arg(item.title).arg(item.subtitle);
            m_resultsList->addItem(displayStr);
        }
    }

    if (m_resultsList->count() > 0) {
        m_resultsList->setCurrentRow(0);
    }
}

void SearchEverywhereDialog::onItemActivated(QListWidgetItem* item) {
    int row = m_resultsList->row(item);
    if (row >= 0 && row < static_cast<int>(m_currentResults.size())) {
        const auto& target = m_currentResults[row];
        emit fileSelected(target.path, target.line);
        accept();
    }
}

void SearchEverywhereDialog::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Down) {
        int next = m_resultsList->currentRow() + 1;
        if (next < m_resultsList->count()) m_resultsList->setCurrentRow(next);
    } else if (event->key() == Qt::Key_Up) {
        int prev = m_resultsList->currentRow() - 1;
        if (prev >= 0) m_resultsList->setCurrentRow(prev);
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (m_resultsList->currentItem()) {
            onItemActivated(m_resultsList->currentItem());
        }
    } else if (event->key() == Qt::Key_Escape) {
        reject();
    } else {
        QDialog::keyPressEvent(event);
    }
}

} // namespace MyIDE::UI
