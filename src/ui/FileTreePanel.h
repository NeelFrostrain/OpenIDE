#pragma once

#include <QWidget>

class QTreeView;
class QFileSystemModel;

// Left-hand "project" panel: shows the currently open folder as a tree and
// emits fileActivated() when the user double-clicks a file so MainWindow can
// open it in a new editor tab.
class FileTreePanel : public QWidget {
    Q_OBJECT
public:
    explicit FileTreePanel(QWidget* parent = nullptr);

    void setRootPath(const QString& path);

signals:
    void fileActivated(const QString& filePath);

private:
    QTreeView* m_treeView;
    QFileSystemModel* m_model;
};
