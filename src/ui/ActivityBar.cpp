#include "ActivityBar.h"
#include <QToolTip>

ActivityBar::ActivityBar(QWidget* parent) : QWidget(parent) {
    setFixedWidth(42);
    setStyleSheet("QWidget { background-color: #2b2d30; border-right: 1px solid #393b40; }"
                  "QPushButton { background: transparent; color: #9da5b4; border: none; font-size: 16px; padding: 10px 0px; margin: 2px 4px; border-radius: 4px; }"
                  "QPushButton:hover { background-color: #35373c; color: #ffffff; }"
                  "QPushButton:checked { background-color: #3574f0; color: #ffffff; }");

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 6, 0, 6);
    m_layout->setSpacing(4);
}

void ActivityBar::addActivity(const QString& iconText, const QString& tooltip, std::function<void()> onClicked) {
    auto* btn = new QPushButton(iconText, this);
    btn->setToolTip(tooltip);
    btn->setCheckable(true);
    connect(btn, &QPushButton::clicked, this, [this, btn, onClicked] {
        btn->setChecked(!btn->isChecked());
        if (onClicked) onClicked();
    });
    m_layout->addWidget(btn);
}
