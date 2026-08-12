#include "ui/ToolWindowHeader.h"

namespace OpenIDE::UI {

ToolWindowHeader::ToolWindowHeader(const QString& title, QWidget* parent)
    : QWidget(parent) {
    setFixedHeight(28);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 2, 6, 2);

    m_titleLabel = new QLabel(title.toUpper(), this);
    m_titleLabel->setFont(QFont("Segoe UI", 8, QFont::Bold));

    m_settingsBtn = new QPushButton("⚙", this);
    m_closeBtn = new QPushButton("✕", this);

    layout->addWidget(m_titleLabel);
    layout->addStretch(1);
    layout->addWidget(m_settingsBtn);
    layout->addWidget(m_closeBtn);

    setStyleSheet(R"(
        QWidget {
            background-color: #252526;
            border-bottom: 1px solid #2D2D2D;
            color: #AAAAAA;
        }
        QLabel {
            color: #AAAAAA;
            letter-spacing: 0.5px;
        }
        QPushButton {
            background: transparent;
            color: #858585;
            border: none;
            font-size: 9pt;
            width: 20px;
            height: 20px;
            border-radius: 3px;
        }
        QPushButton:hover {
            color: #FFFFFF;
            background-color: #37373D;
        }
    )");

    connect(m_settingsBtn, &QPushButton::clicked, this, &ToolWindowHeader::settingsClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &ToolWindowHeader::closeClicked);
}

void ToolWindowHeader::setTitle(const QString& title) {
    m_titleLabel->setText(title.toUpper());
}

} // namespace OpenIDE::UI
