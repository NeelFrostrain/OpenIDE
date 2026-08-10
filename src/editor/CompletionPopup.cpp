#include "editor/CompletionPopup.h"
#include <QPainter>
#include <QKeyEvent>

namespace MyIDE::Editor {

CompletionPopup::CompletionPopup(QWidget* parent)
    : QWidget(parent) {
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::NoFocus);

    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_listWidget = new QListWidget(this);
    m_listWidget->setItemDelegate(new CompletionItemDelegate(this));
    m_listWidget->setFixedWidth(380);

    m_docPanel = new QWidget(this);
    m_docPanel->setFixedWidth(300);
    auto* docLayout = new QVBoxLayout(m_docPanel);
    docLayout->setContentsMargins(10, 10, 10, 10);

    m_docTitle = new QLabel(this);
    m_docTitle->setFont(QFont("Consolas", 10, QFont::Bold));
    m_docTitle->setWordWrap(true);
    m_docTitle->setStyleSheet("color: #DCDCAA;");

    m_docBody = new QLabel(this);
    m_docBody->setFont(QFont("Segoe UI", 9));
    m_docBody->setWordWrap(true);
    m_docBody->setStyleSheet("color: #CCCCCC;");

    docLayout->addWidget(m_docTitle);
    docLayout->addWidget(m_docBody);
    docLayout->addStretch(1);

    m_docPanel->setStyleSheet("background-color: #1E1E1E; border-left: 1px solid #3E3E42;");
    m_docPanel->hide();

    mainLayout->addWidget(m_listWidget);
    mainLayout->addWidget(m_docPanel);

    setStyleSheet(R"(
        QWidget {
            background-color: #252526;
            border: 1px solid #454545;
            color: #CCCCCC;
            font-family: 'Consolas', 'Cascadia Code', monospace;
            font-size: 10pt;
        }
        QListWidget {
            background-color: #252526;
            border: none;
        }
        QListWidget::item:selected {
            background-color: #04395E;
            color: #FFFFFF;
        }
        QListWidget::item:hover {
            background-color: #2A2D2E;
        }
    )");

    connect(m_listWidget, &QListWidget::itemSelectionChanged, this, &CompletionPopup::onSelectionChanged);
    connect(m_listWidget, &QListWidget::itemActivated, [this](QListWidgetItem* item) {
        if (item) {
            emit itemSelected(currentItemData());
            hide();
        }
    });
}

void CompletionPopup::setCompletions(const std::vector<CompletionItemData>& items) {
    m_allItems = items;
    filter("");
}

void CompletionPopup::filter(const QString& prefix) {
    m_listWidget->clear();
    for (const auto& item : m_allItems) {
        if (prefix.isEmpty() || item.label.contains(prefix, Qt::CaseInsensitive)) {
            auto* widgetItem = new QListWidgetItem(item.label, m_listWidget);
            widgetItem->setData(Qt::UserRole, item.detail);
            widgetItem->setData(Qt::UserRole + 1, item.documentation);
            widgetItem->setData(Qt::UserRole + 2, item.insertText);
            widgetItem->setData(Qt::UserRole + 3, item.kind);
            widgetItem->setData(Qt::UserRole + 4, item.isSnippet);
        }
    }
    if (m_listWidget->count() > 0) {
        m_listWidget->setCurrentRow(0);
        setFixedHeight(qMin(240, m_listWidget->count() * 24 + 4));
    }
}

void CompletionPopup::moveSelectionUp() {
    int row = m_listWidget->currentRow();
    if (row > 0) {
        m_listWidget->setCurrentRow(row - 1);
    }
}

void CompletionPopup::moveSelectionDown() {
    int row = m_listWidget->currentRow();
    if (row < m_listWidget->count() - 1) {
        m_listWidget->setCurrentRow(row + 1);
    }
}

void CompletionPopup::moveSelectionPageUp() {
    int row = qMax(0, m_listWidget->currentRow() - 5);
    m_listWidget->setCurrentRow(row);
}

void CompletionPopup::moveSelectionPageDown() {
    int row = qMin(m_listWidget->count() - 1, m_listWidget->currentRow() + 5);
    m_listWidget->setCurrentRow(row);
}

void CompletionPopup::selectHome() {
    if (m_listWidget->count() > 0) {
        m_listWidget->setCurrentRow(0);
    }
}

void CompletionPopup::selectEnd() {
    if (m_listWidget->count() > 0) {
        m_listWidget->setCurrentRow(m_listWidget->count() - 1);
    }
}

bool CompletionPopup::hasItems() const {
    return m_listWidget->count() > 0;
}

CompletionItemData CompletionPopup::currentItemData() const {
    auto* item = m_listWidget->currentItem();
    if (!item) return {};

    CompletionItemData itemData;
    itemData.label = item->text();
    itemData.detail = item->data(Qt::UserRole).toString();
    itemData.documentation = item->data(Qt::UserRole + 1).toString();
    itemData.insertText = item->data(Qt::UserRole + 2).toString();
    itemData.kind = item->data(Qt::UserRole + 3).toInt();
    itemData.isSnippet = item->data(Qt::UserRole + 4).toBool();
    return itemData;
}

void CompletionPopup::onSelectionChanged() {
    auto data = currentItemData();
    if (!data.documentation.isEmpty() || !data.detail.isEmpty()) {
        m_docTitle->setText(data.label + (data.detail.isEmpty() ? "" : " — " + data.detail));
        m_docBody->setText(data.documentation.isEmpty() ? "No additional documentation available." : data.documentation);
        m_docPanel->show();
    } else {
        m_docPanel->hide();
    }
}

void CompletionPopup::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Down || event->key() == Qt::Key_Up || event->key() == Qt::Key_PageDown || event->key() == Qt::Key_PageUp) {
        QCoreApplication::sendEvent(m_listWidget, event);
    } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Tab) {
        if (m_listWidget->currentItem()) {
            emit itemSelected(currentItemData());
            hide();
        }
    } else if (event->key() == Qt::Key_Escape) {
        hide();
    } else {
        QWidget::keyPressEvent(event);
    }
}

CompletionItemDelegate::CompletionItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

void CompletionItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    painter->save();

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor("#04395E"));
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, QColor("#2A2D2E"));
    } else {
        painter->fillRect(option.rect, QColor("#252526"));
    }

    QString label = index.data(Qt::DisplayRole).toString();
    QString detail = index.data(Qt::UserRole).toString();
    int kind = index.data(Qt::UserRole + 3).toInt();

    // Draw Kind Icon Indicator
    QRect iconRect(option.rect.left() + 6, option.rect.top() + 4, 16, 16);
    QColor iconColor("#569CD6");
    if (kind == 2 || kind == 3) iconColor = QColor("#DCDCAA"); // Method / Function
    else if (kind == 6 || kind == 7) iconColor = QColor("#4EC9B0"); // Class / Interface
    else if (kind == 5 || kind == 10) iconColor = QColor("#9CDCFE"); // Field / Variable / Property
    else if (kind == 13 || kind == 14) iconColor = QColor("#C586C0"); // Enum / Struct / Macro

    painter->setPen(Qt::NoPen);
    painter->setBrush(iconColor);
    painter->drawEllipse(iconRect.adjusted(2, 2, -2, -2));

    // Draw Symbol Label
    painter->setPen(QColor("#D4D4D4"));
    QFont labelFont = option.font;
    labelFont.setBold(true);
    painter->setFont(labelFont);
    QRect labelRect(iconRect.right() + 8, option.rect.top(), 200, option.rect.height());
    painter->drawText(labelRect, Qt::AlignVCenter | Qt::AlignLeft, label);

    // Draw Symbol Detail (Type / Class owner)
    if (!detail.isEmpty()) {
        painter->setPen(QColor("#808080"));
        QFont detailFont = option.font;
        detailFont.setPointSize(detailFont.pointSize() - 1);
        painter->setFont(detailFont);
        QRect detailRect(labelRect.right() + 10, option.rect.top(), option.rect.right() - labelRect.right() - 15, option.rect.height());
        painter->drawText(detailRect, Qt::AlignVCenter | Qt::AlignRight, detail);
    }

    painter->restore();
}

QSize CompletionItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(380, 24);
}

} // namespace MyIDE::Editor
