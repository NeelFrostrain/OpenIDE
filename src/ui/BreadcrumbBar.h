#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <filesystem>
#include <vector>
#include <QString>

namespace OpenIDE::UI {

struct BreadcrumbItem {
    QString name;
    QString type; // "folder", "file", "class", "function"
    std::filesystem::path path;
};

class BreadcrumbBar : public QWidget {
    Q_OBJECT

public:
    explicit BreadcrumbBar(QWidget* parent = nullptr);

    void setPathAndSymbol(const std::filesystem::path& filePath, const QString& className = "", const QString& functionName = "");

signals:
    void itemClicked(const BreadcrumbItem& item);

private:
    QHBoxLayout* m_layout = nullptr;
};

} // namespace OpenIDE::UI
