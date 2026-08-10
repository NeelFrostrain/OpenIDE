#pragma once

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

class StatusBar : public QWidget {
    Q_OBJECT
public:
    explicit StatusBar(QWidget* parent = nullptr);

    void setCursorPosition(int line, int col);
    void setLspStatus(const QString& status, bool ok = true);
    void setGitBranch(const QString& branch);

private:
    QLabel* m_branchLabel;
    QLabel* m_lspLabel;
    QLabel* m_langLabel;
    QLabel* m_encodingLabel;
    QLabel* m_posLabel;
};
