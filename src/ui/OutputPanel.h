#pragma once

#include <QPlainTextEdit>

// Bottom panel used for build output / console messages. Read-only text log
// for the prototype; BuildRunner will stream compiler output into this via
// appendLine() in a later milestone.
class OutputPanel : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit OutputPanel(QWidget* parent = nullptr);

public slots:
    void appendLine(const QString& line);
};
