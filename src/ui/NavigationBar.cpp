#include "NavigationBar.h"
#include <QFileInfo>
#include <QDir>

NavigationBar::NavigationBar(QWidget* parent) : QWidget(parent) {
    setFixedHeight(28);
    setStyleSheet("QWidget { background-color: #1e1f22; border-bottom: 1px solid #2b2d30; }"
                  "QPushButton { color: #9da5b4; border: none; background: transparent; padding: 2px 6px; font-size: 12px; }"
                  "QPushButton:hover { color: #ffffff; background-color: #2b2d30; border-radius: 3px; }"
                  "QLabel { color: #5c6370; font-size: 12px; }");

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 0, 8, 0);
    m_layout->setSpacing(4);
    m_layout->addStretch();
}

void NavigationBar::setPath(const QString& projectPath, const QString& filePath, const QString& currentSymbol) {
    QLayoutItem* child;
    while ((child = m_layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    if (filePath.isEmpty()) {
        m_layout->addStretch();
        return;
    }

    QString relPath = QDir(projectPath).relativeFilePath(filePath);
    QStringList parts = relPath.split('/', Qt::SkipEmptyParts);

    auto* projBtn = new QPushButton(QFileInfo(projectPath).fileName(), this);
    m_layout->addWidget(projBtn);

    for (int i = 0; i < parts.size(); ++i) {
        auto* sep = new QLabel("›", this);
        m_layout->addWidget(sep);

        auto* btn = new QPushButton(parts[i], this);
        if (i == parts.size() - 1) {
            btn->setStyleSheet("font-weight: bold; color: #d19a66;");
        }
        m_layout->addWidget(btn);
    }

    if (!currentSymbol.isEmpty()) {
        auto* sep = new QLabel("›", this);
        m_layout->addWidget(sep);

        auto* symBtn = new QPushButton(currentSymbol, this);
        symBtn->setStyleSheet("color: #61afef; font-weight: bold;");
        m_layout->addWidget(symBtn);
    }

    m_layout->addStretch();
}
