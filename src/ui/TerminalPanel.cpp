#include "TerminalPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDir>

TerminalPanel::TerminalPanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    m_outputView = new QTextEdit(this);
    m_outputView->setReadOnly(true);
    m_outputView->setFontFamily("Cascadia Code");
    m_outputView->setStyleSheet("background-color: #18191c; color: #abb2bf; border: 1px solid #2b2d30;");
    layout->addWidget(m_outputView);

    auto* inputLayout = new QHBoxLayout();
    auto* promptLabel = new QLabel("PS >", this);
    promptLabel->setStyleSheet("color: #61afef; font-weight: bold; padding-left: 4px;");
    inputLayout->addWidget(promptLabel);

    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setStyleSheet("background-color: #21252b; color: #dfe1e5; border: 1px solid #393b40; padding: 4px; font-family: 'Cascadia Code';");
    connect(m_inputEdit, &QLineEdit::returnPressed, this, &TerminalPanel::onCommandEntered);
    inputLayout->addWidget(m_inputEdit);

    layout->addLayout(inputLayout);

    connect(&m_process, &QProcess::readyReadStandardOutput, this, &TerminalPanel::onReadyReadStandardOutput);
    connect(&m_process, &QProcess::readyReadStandardError, this, &TerminalPanel::onReadyReadStandardError);

    m_process.setWorkingDirectory(QDir::currentPath());
    m_process.start("powershell.exe", {"-NoLogo", "-NoExit", "-Command", "-"});
}

TerminalPanel::~TerminalPanel() {
    if (m_process.state() != QProcess::NotRunning) {
        m_process.terminate();
        m_process.waitForFinished(1000);
    }
}

void TerminalPanel::onCommandEntered() {
    QString cmd = m_inputEdit->text().trimmed();
    if (cmd.isEmpty()) return;

    m_outputView->append("<span style='color:#61afef; font-weight:bold;'>PS > " + cmd.toHtmlEscaped() + "</span>");
    m_inputEdit->clear();

    m_process.write((cmd + "\r\n").toUtf8());
}

void TerminalPanel::onReadyReadStandardOutput() {
    QString out = QString::fromUtf8(m_process.readAllStandardOutput());
    m_outputView->append(out.toHtmlEscaped());
}

void TerminalPanel::onReadyReadStandardError() {
    QString err = QString::fromUtf8(m_process.readAllStandardError());
    m_outputView->append("<span style='color:#e06c75;'>" + err.toHtmlEscaped() + "</span>");
}
