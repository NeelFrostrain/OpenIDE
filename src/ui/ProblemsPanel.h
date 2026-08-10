#pragma once

#include <QWidget>
#include <QTreeWidget>
#include "lsp/LspTypes.h"

class ProblemsPanel : public QWidget {
    Q_OBJECT
public:
    explicit ProblemsPanel(QWidget* parent = nullptr);

    void setDiagnostics(const QString& filePath, const QList<LspDiagnostic>& diagnostics);
    void clearAllDiagnostics();

signals:
    void problemSelected(const QString& filePath, int line, int character);

private slots:
    void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
    QTreeWidget* m_treeWidget;
    QMap<QString, QTreeWidgetItem*> m_fileNodes;
};
