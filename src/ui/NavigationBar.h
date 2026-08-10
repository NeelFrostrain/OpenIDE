#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

class NavigationBar : public QWidget {
    Q_OBJECT
public:
    explicit NavigationBar(QWidget* parent = nullptr);

    void setPath(const QString& projectPath, const QString& filePath, const QString& currentSymbol = "");

signals:
    void pathSegmentClicked(const QString& path);

private:
    QHBoxLayout* m_layout;
};
