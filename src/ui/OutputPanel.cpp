#include "OutputPanel.h"

OutputPanel::OutputPanel(QWidget* parent) : QPlainTextEdit(parent) {
    setReadOnly(true);
    setMaximumBlockCount(5000);
}

void OutputPanel::appendLine(const QString& line) {
    appendPlainText(line);
}
