#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

class ActivityBar : public QWidget {
    Q_OBJECT
public:
    explicit ActivityBar(QWidget* parent = nullptr);

    void addActivity(const QString& iconText, const QString& tooltip, std::function<void()> onClicked);

private:
    QVBoxLayout* m_layout;
};
