#include "TitleBar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMouseEvent>

TitleBar::TitleBar(QWidget* parent) : QWidget(parent) {
    setObjectName("TitleBar");
    setFixedHeight(36);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_titleLabel = new QLabel("MyIDE", this);
    m_titleLabel->setObjectName("appTitle");

    m_minButton = new QPushButton("\u2013", this);   // –
    m_maxButton = new QPushButton("\u25A1", this);    // □
    m_closeButton = new QPushButton("\u00D7", this);  // ×

    for (auto* btn : {m_minButton, m_maxButton, m_closeButton}) {
        btn->setObjectName("titleBarBtn");
        btn->setFixedSize(46, 36);
        btn->setCursor(Qt::ArrowCursor);
    }
    m_closeButton->setObjectName("closeBtn");

    layout->addSpacing(4);
    layout->addWidget(m_titleLabel);
    layout->addStretch(1);
    layout->addWidget(m_minButton);
    layout->addWidget(m_maxButton);
    layout->addWidget(m_closeButton);

    connect(m_minButton, &QPushButton::clicked, this, &TitleBar::minimizeRequested);
    connect(m_maxButton, &QPushButton::clicked, this, &TitleBar::maximizeRestoreRequested);
    connect(m_closeButton, &QPushButton::clicked, this, &TitleBar::closeRequested);
}

void TitleBar::setTitle(const QString& title) {
    m_titleLabel->setText(title);
}

void TitleBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragOffset = event->globalPosition().toPoint() - window()->pos();
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent* event) {
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        window()->move(event->globalPosition().toPoint() - m_dragOffset);
    }
    QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit maximizeRestoreRequested();
    }
    QWidget::mouseDoubleClickEvent(event);
}
