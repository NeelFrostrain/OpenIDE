#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include "test/TestRunner.h"

class TestRunnerPanel : public QWidget {
    Q_OBJECT
public:
    explicit TestRunnerPanel(QWidget* parent = nullptr);

    void setTestBinary(const QString& binaryPath);

private slots:
    void onRunClicked();

private:
    TestRunner* m_runner;
    QString m_binaryPath;
    QTreeWidget* m_treeWidget;
    QPushButton* m_runBtn;
};
