#include "ProblemsPanel.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFileInfo>

ProblemsPanel::ProblemsPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({tr("Problem"), tr("File"), tr("Line")});
    m_treeWidget->header()->resizeSection(0, 450);
    m_treeWidget->header()->resizeSection(1, 200);

    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &ProblemsPanel::onItemDoubleClicked);

    layout->addWidget(m_treeWidget);
}

void ProblemsPanel::clearAllDiagnostics() {
    m_treeWidget->clear();
    m_fileNodes.clear();
}

void ProblemsPanel::setDiagnostics(const QString& filePath, const QList<LspDiagnostic>& diagnostics) {
    if (m_fileNodes.contains(filePath)) {
        delete m_fileNodes.take(filePath);
    }

    if (diagnostics.isEmpty()) return;

    auto* fileItem = new QTreeWidgetItem(m_treeWidget);
    fileItem->setText(0, QFileInfo(filePath).fileName());
    fileItem->setData(0, Qt::UserRole, filePath);
    fileItem->setExpanded(true);

    m_fileNodes[filePath] = fileItem;

    for (const auto& d : diagnostics) {
        auto* item = new QTreeWidgetItem(fileItem);
        QString prefix = (d.severity == LspDiagnosticSeverity::Error) ? tr("[Error] ") : tr("[Warning] ");
        item->setText(0, prefix + d.message);
        item->setText(1, QFileInfo(filePath).fileName());
        item->setText(2, QString::number(d.range.start.line + 1));

        item->setData(0, Qt::UserRole, filePath);
        item->setData(1, Qt::UserRole, d.range.start.line);
        item->setData(2, Qt::UserRole, d.range.start.character);
    }
}

void ProblemsPanel::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    if (!item || !item->parent()) return; // Ignore root file items

    QString filePath = item->data(0, Qt::UserRole).toString();
    int line = item->data(1, Qt::UserRole).toInt();
    int col = item->data(2, Qt::UserRole).toInt();

    emit problemSelected(filePath, line, col);
}
