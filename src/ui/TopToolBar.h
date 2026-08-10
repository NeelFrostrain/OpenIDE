#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>

namespace MyIDE::UI {

class TopToolBar : public QWidget {
    Q_OBJECT

public:
    explicit TopToolBar(QWidget* parent = nullptr);

signals:
    void buildRequested();
    void runRequested();
    void debugRequested();

private:
    QComboBox* m_configCombo = nullptr;
    QComboBox* m_platformCombo = nullptr;
    QComboBox* m_targetCombo = nullptr;
    QPushButton* m_buildBtn = nullptr;
    QPushButton* m_runBtn = nullptr;
    QPushButton* m_debugBtn = nullptr;
};

} // namespace MyIDE::UI
