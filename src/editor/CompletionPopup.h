#pragma once

#include "editor/ITextEditor.h"
#include <QListWidget>
#include <QStyledItemDelegate>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <vector>

namespace MyIDE::Editor {

class CompletionPopup : public QWidget {
    Q_OBJECT

public:
    explicit CompletionPopup(QWidget* parent = nullptr);

    void setCompletions(const std::vector<CompletionItemData>& items);
    void filter(const QString& prefix);

    CompletionItemData currentItemData() const;
    bool hasItems() const;

signals:
    void itemSelected(const CompletionItemData& item);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onSelectionChanged();

private:
    QListWidget* m_listWidget = nullptr;
    QWidget* m_docPanel = nullptr;
    QLabel* m_docTitle = nullptr;
    QLabel* m_docBody = nullptr;

    std::vector<CompletionItemData> m_allItems;
};

class CompletionItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit CompletionItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

} // namespace MyIDE::Editor
