#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QProcess>

class TerminalPanel : public QWidget {
    Q_OBJECT
public:
    explicit TerminalPanel(QWidget* parent = nullptr);
    ~TerminalPanel() override;

private slots:
    void onCommandEntered();
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();

private:
    QTextEdit* m_outputView;
    QLineEdit* m_inputEdit;
    QProcess m_process;
};
