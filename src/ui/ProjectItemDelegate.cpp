#include "ui/ProjectItemDelegate.h"
#include <QPainter>
#include <QFileSystemModel>

namespace OpenIDE::UI {

ProjectItemDelegate::ProjectItemDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

void ProjectItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    painter->save();

    // Background selection & hover state
    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, QColor("#37373D"));
    } else if (option.state & QStyle::State_MouseOver) {
        painter->fillRect(option.rect, QColor("#2A2D2E"));
    }

    QString text = index.data(Qt::DisplayRole).toString();
    const auto* model = qobject_cast<const QFileSystemModel*>(index.model());
    bool isDir = model ? model->isDir(index) : false;

    // Determine File / Icon Badge
    QString iconBadge = "📄";
    QColor badgeColor("#9CDCFE");

    if (isDir) {
        iconBadge = "📁";
        badgeColor = QColor("#DCB67A");
    } else if (text.endsWith(".cpp") || text.endsWith(".c")) {
        iconBadge = "C";
        badgeColor = QColor("#569CD6");
    } else if (text.endsWith(".h") || text.endsWith(".hpp")) {
        iconBadge = "H";
        badgeColor = QColor("#C586C0");
    } else if (text.endsWith(".uproject")) {
        iconBadge = "U";
        badgeColor = QColor("#007ACC");
    } else if (text.contains(".Build.cs") || text.contains(".Target.cs")) {
        iconBadge = "CS";
        badgeColor = QColor("#4EC9B0");
    }

    // De-emphasize generated folders
    bool isDeemphasized = (text == "Binaries" || text == "Intermediate" || text == "DerivedDataCache" || text == "Saved" || text == ".ide" || text == ".git");
    QColor textColor = isDeemphasized ? QColor("#666666") : QColor("#CCCCCC");

    // Draw Icon Badge
    QRect iconRect(option.rect.left() + 4, option.rect.top() + 3, 16, 16);
    if (!isDir && (iconBadge == "C" || iconBadge == "H" || iconBadge == "U" || iconBadge == "CS")) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(badgeColor);
        painter->drawRoundedRect(iconRect, 3, 3);

        painter->setPen(QColor("#FFFFFF"));
        QFont badgeFont = option.font;
        badgeFont.setPointSize(7);
        badgeFont.setBold(true);
        painter->setFont(badgeFont);
        painter->drawText(iconRect, Qt::AlignCenter, iconBadge);
    } else {
        painter->setPen(badgeColor);
        painter->drawText(iconRect, Qt::AlignCenter, iconBadge);
    }

    // Draw Item Name
    painter->setPen(textColor);
    QFont textFont = option.font;
    textFont.setPointSize(9);
    painter->setFont(textFont);
    QRect textRect(iconRect.right() + 6, option.rect.top(), option.rect.width() - 26, option.rect.height());
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

    painter->restore();
}

QSize ProjectItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(220, 22);
}

} // namespace OpenIDE::UI
