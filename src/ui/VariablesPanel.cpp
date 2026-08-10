#include "VariablesPanel.h"
#include <QVBoxLayout>
#include <QHeaderView>

VariablesPanel::VariablesPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({tr("Variable"), tr("Value"), tr("Type")});
    m_treeWidget->header()->resizeSection(0, 160);
    m_treeWidget->header()->resizeSection(1, 160);
    layout->addWidget(m_treeWidget);
}

void VariablesPanel::setVariables(const QList<VariableItem>& vars) {
    m_treeWidget->clear();
    for (const auto& v : vars) {
        auto* item = new QTreeWidgetItem(m_treeWidget);
        item->setText(0, v.name);
        item->setText(1, v.value);
        item->setText(2, v.type);
    }
}
