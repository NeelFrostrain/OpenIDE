#include "CallStackPanel.h"
#include <QVBoxLayout>
#include <QFileInfo>

CallStackPanel::CallStackPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_listWidget = new QListWidget(this);
    connect(m_listWidget, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem* item) {
        if (!item) return;
        QString path = item->data(Qt::UserRole).toString();
        int line = item->data(Qt::UserRole + 1).toInt();
        emit frameSelected(path, line);
    });

    layout->addWidget(m_listWidget);
}

void CallStackPanel::setCallStack(const QList<StackFrameItem>& frames) {
    m_listWidget->clear();
    for (const auto& f : frames) {
        auto* item = new QListWidgetItem(m_listWidget);
        item->setText(QString("%1() — %2:%3").arg(f.name, QFileInfo(f.filePath).fileName()).arg(f.line + 1));
        item->setData(Qt::UserRole, f.filePath);
        item->setData(Qt::UserRole + 1, f.line);
    }
}
