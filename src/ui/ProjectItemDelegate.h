#pragma once

#include <QStyledItemDelegate>

namespace OpenIDE::UI {

class ProjectItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit ProjectItemDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

} // namespace OpenIDE::UI
