#include "ui/BreadcrumbBar.h"
#include <QStyle>

namespace MyIDE::UI {

BreadcrumbBar::BreadcrumbBar(QWidget* parent)
    : QWidget(parent) {
    setFixedHeight(26);
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 2, 8, 2);
    m_layout->setSpacing(4);

    setStyleSheet(R"(
        QWidget {
            background-color: #252526;
            color: #CCCCCC;
            font-family: 'Segoe UI', sans-serif;
            font-size: 9pt;
        }
        QPushButton {
            background: transparent;
            color: #9CDCFE;
            border: none;
            padding: 2px 4px;
            border-radius: 3px;
        }
        QPushButton:hover {
            background-color: #37373D;
            color: #FFFFFF;
        }
        QLabel {
            color: #666666;
            font-weight: bold;
        }
    )");
}

void BreadcrumbBar::setPathAndSymbol(const std::filesystem::path& filePath, const QString& className, const QString& functionName) {
    // Clear old breadcrumbs
    QLayoutItem* child;
    while ((child = m_layout->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    if (filePath.empty()) return;

    std::vector<QString> parts;
    std::filesystem::path rel = filePath.filename();
    parts.push_back(QString::fromStdString(filePath.parent_path().filename().string()));
    parts.push_back(QString::fromStdString(filePath.filename().string()));

    if (!className.isEmpty()) parts.push_back(className);
    if (!functionName.isEmpty()) parts.push_back(functionName + "()");

    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) {
            auto* sep = new QLabel(" › ", this);
            m_layout->addWidget(sep);
        }

        auto* btn = new QPushButton(parts[i], this);
        if (i == parts.size() - 1) {
            btn->setStyleSheet("color: #FFFFFF; font-weight: bold;");
        }
        m_layout->addWidget(btn);
    }

    m_layout->addStretch(1);
}

} // namespace MyIDE::UI
