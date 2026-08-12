#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QButtonGroup>

namespace OpenIDE::UI {

enum class ToolWindowTab {
    Project,
    Search,
    Git,
    Run,
    Debug
};

class LeftToolBar : public QWidget {
    Q_OBJECT

public:
    explicit LeftToolBar(QWidget* parent = nullptr);

signals:
    void tabToggled(ToolWindowTab tab, bool visible);

private:
    QVBoxLayout* m_layout = nullptr;
    QButtonGroup* m_group = nullptr;

    QPushButton* createToolButton(const QString& iconText, const QString& tooltip, ToolWindowTab tab);
};

} // namespace OpenIDE::UI
