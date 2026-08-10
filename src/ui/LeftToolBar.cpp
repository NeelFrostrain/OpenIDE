#include "ui/LeftToolBar.h"

namespace MyIDE::UI {

LeftToolBar::LeftToolBar(QWidget* parent)
    : QWidget(parent) {
    setFixedWidth(46);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 4, 0, 4);
    m_layout->setSpacing(4);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(false);

    m_layout->addWidget(createToolButton("▣", "Project Explorer", ToolWindowTab::Project));
    m_layout->addWidget(createToolButton("⌕", "Search Everywhere", ToolWindowTab::Search));
    m_layout->addWidget(createToolButton("⑂", "Git Source Control", ToolWindowTab::Git));
    m_layout->addWidget(createToolButton("▶", "Run / Build", ToolWindowTab::Run));
    m_layout->addWidget(createToolButton("🐞", "Debugger", ToolWindowTab::Debug));

    m_layout->addStretch(1);

    setStyleSheet(R"(
        QWidget {
            background-color: #1B1B1C;
            border-right: 1px solid #2D2D2D;
        }
        QPushButton {
            background: transparent;
            color: #858585;
            border: none;
            border-left: 2px solid transparent;
            font-size: 14pt;
            min-height: 40px;
            max-height: 40px;
        }
        QPushButton:hover {
            color: #CCCCCC;
            background-color: #2A2D2E;
        }
        QPushButton:checked {
            color: #FFFFFF;
            border-left: 2px solid #007ACC;
            background-color: #252526;
        }
    )");
}

QPushButton* LeftToolBar::createToolButton(const QString& iconText, const QString& tooltip, ToolWindowTab tab) {
    auto* btn = new QPushButton(iconText, this);
    btn->setToolTip(tooltip);
    btn->setCheckable(true);
    if (tab == ToolWindowTab::Project) btn->setChecked(true);

    connect(btn, &QPushButton::toggled, [this, tab](bool checked) {
        emit tabToggled(tab, checked);
    });

    return btn;
}

} // namespace MyIDE::UI
