#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

namespace OpenIDE::UI {

class ToolWindowHeader : public QWidget {
    Q_OBJECT

public:
    explicit ToolWindowHeader(const QString& title, QWidget* parent = nullptr);

    void setTitle(const QString& title);

signals:
    void settingsClicked();
    void closeClicked();

private:
    QLabel* m_titleLabel = nullptr;
    QPushButton* m_settingsBtn = nullptr;
    QPushButton* m_closeBtn = nullptr;
};

} // namespace OpenIDE::UI
