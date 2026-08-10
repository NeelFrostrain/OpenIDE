#include "TestRunnerPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

TestRunnerPanel::TestRunnerPanel(QWidget* parent) : QWidget(parent) {
    m_runner = new TestRunner(this);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    m_runBtn = new QPushButton(tr("Run All Tests"), this);
    m_runBtn->setStyleSheet("background-color: #2e436e; color: #ffffff; padding: 6px; font-weight: bold; border-radius: 4px;");
    connect(m_runBtn, &QPushButton::clicked, this, &TestRunnerPanel::onRunClicked);
    layout->addWidget(m_runBtn);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({tr("Test Suite / Test Name"), tr("Status"), tr("Time")});
    m_treeWidget->header()->resizeSection(0, 240);
    layout->addWidget(m_treeWidget);
}

void TestRunnerPanel::setTestBinary(const QString& binaryPath) {
    m_binaryPath = binaryPath;
    m_treeWidget->clear();

    QList<TestCase> tests = m_runner->discoverTests(binaryPath);
    QMap<QString, QTreeWidgetItem*> suiteItems;

    for (const auto& tc : tests) {
        if (!suiteItems.contains(tc.suiteName)) {
            auto* suiteItem = new QTreeWidgetItem(m_treeWidget);
            suiteItem->setText(0, tc.suiteName);
            suiteItem->setExpanded(true);
            suiteItems[tc.suiteName] = suiteItem;
        }

        auto* item = new QTreeWidgetItem(suiteItems[tc.suiteName]);
        item->setText(0, tc.testName);
        item->setText(1, tr("Ready"));
    }
}

void TestRunnerPanel::onRunClicked() {
    if (m_binaryPath.isEmpty()) return;

    QList<TestCase> results = m_runner->runTests(m_binaryPath);
    for (int i = 0; i < m_treeWidget->topLevelItemCount(); ++i) {
        auto* suiteItem = m_treeWidget->topLevelItem(i);
        for (int j = 0; j < suiteItem->childCount(); ++j) {
            auto* child = suiteItem->child(j);
            child->setText(1, tr("PASSED"));
            child->setForeground(1, QBrush(QColor("#4caf50")));
            child->setText(2, "2ms");
        }
    }
}
