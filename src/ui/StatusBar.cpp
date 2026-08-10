#include "StatusBar.h"

StatusBar::StatusBar(QWidget* parent) : QWidget(parent) {
    setFixedHeight(24);
    setStyleSheet("QWidget { background-color: #2b2d30; border-top: 1px solid #393b40; color: #9da5b4; font-size: 11px; }"
                  "QLabel { color: #9da5b4; padding: 0 8px; }"
                  "QLabel#branch { color: #98c379; font-weight: bold; }"
                  "QLabel#lsp { color: #61afef; }");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(4);

    m_branchLabel = new QLabel("🌿 main", this);
    m_branchLabel->setObjectName("branch");
    layout->addWidget(m_branchLabel);

    m_lspLabel = new QLabel("🟢 clangd connected", this);
    m_lspLabel->setObjectName("lsp");
    layout->addWidget(m_lspLabel);

    layout->addStretch();

    m_langLabel = new QLabel("C++", this);
    layout->addWidget(m_langLabel);

    m_encodingLabel = new QLabel("UTF-8", this);
    layout->addWidget(m_encodingLabel);

    m_posLabel = new QLabel("Ln 1, Col 1", this);
    layout->addWidget(m_posLabel);
}

void StatusBar::setCursorPosition(int line, int col) {
    m_posLabel->setText(QString("Ln %1, Col %2").arg(line).arg(col));
}

void StatusBar::setLspStatus(const QString& status, bool ok) {
    m_lspLabel->setText(QString("%1 %2").arg(ok ? "🟢" : "🔴", status));
}

void StatusBar::setGitBranch(const QString& branch) {
    m_branchLabel->setText(QString("🌿 %1").arg(branch));
}
