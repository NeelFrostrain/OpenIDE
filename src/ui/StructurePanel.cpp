#include "StructurePanel.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFileInfo>

StructurePanel::StructurePanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({tr("Structure"), tr("Kind")});
    m_treeWidget->header()->resizeSection(0, 180);

    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &StructurePanel::onItemDoubleClicked);

    layout->addWidget(m_treeWidget);
}

void StructurePanel::clearStructure() {
    m_treeWidget->clear();
    m_currentFilePath.clear();
}

void StructurePanel::setFileSymbols(const QString& filePath, const QList<SymbolItem>& symbols) {
    m_treeWidget->clear();
    m_currentFilePath = filePath;

    if (symbols.isEmpty()) return;

    for (const auto& sym : symbols) {
        if (sym.kind == "file") continue;

        auto* item = new QTreeWidgetItem(m_treeWidget);
        item->setText(0, sym.name);
        item->setText(1, sym.kind.toUpper());

        item->setData(0, Qt::UserRole, filePath);
        item->setData(1, Qt::UserRole, sym.line);
        item->setData(2, Qt::UserRole, sym.character);
    }

    m_treeWidget->expandAll();
}

void StructurePanel::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    if (!item) return;

    QString filePath = item->data(0, Qt::UserRole).toString();
    int line = item->data(1, Qt::UserRole).toInt();
    int col = item->data(2, Qt::UserRole).toInt();

    emit symbolSelected(filePath, line, col);
}
