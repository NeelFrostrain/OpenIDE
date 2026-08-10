#include "CMakeBar.h"
#include <QHBoxLayout>

CMakeBar::CMakeBar(QWidget* parent) : QWidget(parent) {
    setFixedHeight(30);
    setStyleSheet("QWidget { background-color: #1e1f22; border-bottom: 1px solid #2b2d30; }"
                  "QComboBox { background-color: #2b2d30; color: #dfe1e5; padding: 2px 6px; border: 1px solid #4e5157; border-radius: 3px; font-size: 12px; }"
                  "QPushButton { background-color: #2b2d30; color: #9da5b4; border: none; padding: 2px 8px; font-size: 12px; }"
                  "QPushButton:hover { color: #ffffff; background-color: #34363a; border-radius: 3px; }");

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(6);

    m_configCombo = new QComboBox(this);
    m_configCombo->addItems({"Debug", "Release", "RelWithDebInfo"});
    layout->addWidget(m_configCombo);

    m_buildBtn = new QPushButton("🔨 Build", this);
    connect(m_buildBtn, &QPushButton::clicked, this, [this] {
        emit buildRequested(m_configCombo->currentText());
    });
    layout->addWidget(m_buildBtn);

    m_cleanBtn = new QPushButton("🧹 Clean", this);
    connect(m_cleanBtn, &QPushButton::clicked, this, &CMakeBar::cleanRequested);
    layout->addWidget(m_cleanBtn);

    m_rebuildBtn = new QPushButton("⚡ Rebuild", this);
    connect(m_rebuildBtn, &QPushButton::clicked, this, &CMakeBar::rebuildRequested);
    layout->addWidget(m_rebuildBtn);

    layout->addStretch();
}
