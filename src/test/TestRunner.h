#pragma once

#include <QObject>
#include <QString>
#include <QList>

struct TestCase {
    QString suiteName;
    QString testName;
    bool passed{true};
    int durationMs{0};
    QString failureMessage;
};

class TestRunner : public QObject {
    Q_OBJECT
public:
    explicit TestRunner(QObject* parent = nullptr);

    QList<TestCase> discoverTests(const QString& testBinaryPath);
    QList<TestCase> runTests(const QString& testBinaryPath);

signals:
    void testFinished(const TestCase& testCase);
};
