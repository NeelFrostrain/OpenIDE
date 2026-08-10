#pragma once

#include <QWidget>
#include <QPushButton>
#include "dap/DapClient.h"

class DebugToolbar : public QWidget {
    Q_OBJECT
public:
    explicit DebugToolbar(DapClient* dapClient, QWidget* parent = nullptr);

private:
    DapClient* m_dapClient;
    QPushButton* m_startBtn;
    QPushButton* m_pauseBtn;
    QPushButton* m_stopBtn;
    QPushButton* m_stepOverBtn;
    QPushButton* m_stepIntoBtn;
    QPushButton* m_stepOutBtn;
};
