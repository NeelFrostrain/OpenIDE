#pragma once

#include <QWidget>
#include <QListWidget>
#include "dap/DapTypes.h"

class CallStackPanel : public QWidget {
    Q_OBJECT
public:
    explicit CallStackPanel(QWidget* parent = nullptr);

    void setCallStack(const QList<StackFrameItem>& frames);

signals:
    void frameSelected(const QString& filePath, int line);

private:
    QListWidget* m_listWidget;
};
