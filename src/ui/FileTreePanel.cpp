#include "FileTreePanel.h"

#include <QTreeView>
#include <QFileSystemModel>
#include <QVBoxLayout>
#include <QDir>

FileTreePanel::FileTreePanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_model = new QFileSystemModel(this);
    m_model->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);

    m_treeView = new QTreeView(this);
    m_treeView->setModel(m_model);
    m_treeView->setHeaderHidden(true);
    m_treeView->setAnimated(true);
    m_treeView->setIndentation(14);
    // Only show the "Name" column; hide size/type/date columns for a clean look.
    for (int col = 1; col < m_model->columnCount(); ++col) {
        m_treeView->hideColumn(col);
    }

    layout->addWidget(m_treeView);

    connect(m_treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) {
        if (!m_model->isDir(index)) {
            emit fileActivated(m_model->filePath(index));
        }
    });
}

void FileTreePanel::setRootPath(const QString& path) {
    QModelIndex rootIndex = m_model->setRootPath(path);
    m_treeView->setRootIndex(rootIndex);
}
