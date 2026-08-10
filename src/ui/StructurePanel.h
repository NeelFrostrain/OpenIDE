#pragma once

#include <QWidget>
#include <QTreeWidget>
#include "core/SymbolIndex.h"

class StructurePanel : public QWidget {
    Q_OBJECT
public:
    explicit StructurePanel(QWidget* parent = nullptr);

    void setFileSymbols(const QString& filePath, const QList<SymbolItem>& symbols);
    void clearStructure();

signals:
    void symbolSelected(const QString& filePath, int line, int character);

private slots:
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    QTreeWidget* m_treeWidget;
    QString m_currentFilePath;
};
