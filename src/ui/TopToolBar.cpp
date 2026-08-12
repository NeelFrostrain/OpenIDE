#include "ui/TopToolBar.h"

namespace OpenIDE::UI {

TopToolBar::TopToolBar(QWidget* parent)
    : QWidget(parent) {
    setFixedHeight(34);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 4, 12, 4);
    layout->setSpacing(8);

    m_configCombo = new QComboBox(this);
    m_configCombo->addItems({"Development Editor", "DebugGame Editor", "Shipping", "Development", "DebugGame"});

    m_platformCombo = new QComboBox(this);
    m_platformCombo->addItems({"Win64", "Linux", "Android"});

    m_targetCombo = new QComboBox(this);
    m_targetCombo->addItems({"MyGameEditor", "MyGame", "UnrealBuildTool"});

    m_buildBtn = new QPushButton("🔨 Build", this);
    m_runBtn = new QPushButton("▶ Run", this);
    m_debugBtn = new QPushButton("🐞 Debug", this);

    layout->addWidget(m_configCombo);
    layout->addWidget(m_platformCombo);
    layout->addWidget(m_targetCombo);
    layout->addWidget(m_buildBtn);
    layout->addWidget(m_runBtn);
    layout->addWidget(m_debugBtn);
    layout->addStretch(1);

    setStyleSheet(R"(
        QWidget {
            background-color: #2D2D2D;
            border-bottom: 1px solid #3E3E42;
            color: #CCCCCC;
            font-family: 'Segoe UI', sans-serif;
            font-size: 9pt;
        }
        QComboBox {
            background-color: #1E1E1E;
            color: #D4D4D4;
            border: 1px solid #3E3E42;
            border-radius: 3px;
            padding: 3px 8px;
            min-width: 130px;
        }
        QComboBox:hover {
            border-color: #007ACC;
        }
        QComboBox QAbstractItemView {
            background-color: #252526;
            color: #CCCCCC;
            selection-background-color: #04395E;
        }
        QPushButton {
            background-color: #333333;
            color: #FFFFFF;
            border: 1px solid #3E3E42;
            border-radius: 3px;
            padding: 4px 12px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #007ACC;
            border-color: #007ACC;
        }
    )");

    connect(m_buildBtn, &QPushButton::clicked, this, &TopToolBar::buildRequested);
    connect(m_runBtn, &QPushButton::clicked, this, &TopToolBar::runRequested);
    connect(m_debugBtn, &QPushButton::clicked, this, &TopToolBar::debugRequested);
}

} // namespace OpenIDE::UI
