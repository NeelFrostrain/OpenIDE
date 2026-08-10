#pragma once

#include <QWidget>
#include <QTreeWidget>
#include "dap/DapTypes.h"

class VariablesPanel : public QWidget {
    Q_OBJECT
public:
    explicit VariablesPanel(QWidget* parent = nullptr);

    void setVariables(const QList<VariableItem>& vars);

private:
    QTreeWidget* m_treeWidget;
};
