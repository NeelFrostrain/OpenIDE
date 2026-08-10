#include "DebugToolbar.h"
#include <QHBoxLayout>

DebugToolbar::DebugToolbar(DapClient* dapClient, QWidget* parent)
    : QWidget(parent), m_dapClient(dapClient) {
    setFixedHeight(32);
    setStyleSheet("QWidget { background-color: #21252b; border-bottom: 1px solid #2b2d30; }"
                  "QPushButton { background: transparent; color: #abb2bf; font-weight: bold; border: none; padding: 4px 8px; }"
                  "QPushButton:hover { background-color: #2c313a; color: #ffffff; border-radius: 3px; }");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(4);

    m_startBtn = new QPushButton("▶ Start (F5)", this);
    m_startBtn->setStyleSheet("color: #98c379; font-weight: bold;");
    connect(m_startBtn, &QPushButton::clicked, this, [this] {
        if (m_dapClient) m_dapClient->startDebugging("");
    });
    layout->addWidget(m_startBtn);

    m_pauseBtn = new QPushButton("⏸ Pause", this);
    connect(m_pauseBtn, &QPushButton::clicked, this, [this] {
        if (m_dapClient) m_dapClient->pause();
    });
    layout->addWidget(m_pauseBtn);

    m_stopBtn = new QPushButton("⏹ Stop", this);
    m_stopBtn->setStyleSheet("color: #e06c75; font-weight: bold;");
    connect(m_stopBtn, &QPushButton::clicked, this, [this] {
        if (m_dapClient) m_dapClient->stopDebugging();
    });
    layout->addWidget(m_stopBtn);

    m_stepOverBtn = new QPushButton("↷ Over (F10)", this);
    connect(m_stepOverBtn, &QPushButton::clicked, this, [this] {
        if (m_dapClient) m_dapClient->stepOver();
    });
    layout->addWidget(m_stepOverBtn);

    m_stepIntoBtn = new QPushButton("↓ Into (F11)", this);
    connect(m_stepIntoBtn, &QPushButton::clicked, this, [this] {
        if (m_dapClient) m_dapClient->stepInto();
    });
    layout->addWidget(m_stepIntoBtn);

    m_stepOutBtn = new QPushButton("↑ Out (Shift+F11)", this);
    connect(m_stepOutBtn, &QPushButton::clicked, this, [this] {
        if (m_dapClient) m_dapClient->stepOut();
    });
    layout->addWidget(m_stepOutBtn);

    layout->addStretch();
}
