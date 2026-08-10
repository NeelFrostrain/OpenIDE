#pragma once

#include <QWidget>
#include <QComboBox>
#include <QPushButton>

class CMakeBar : public QWidget {
    Q_OBJECT
public:
    explicit CMakeBar(QWidget* parent = nullptr);

signals:
    void buildRequested(const QString& configMode);
    void cleanRequested();
    void rebuildRequested();

private:
    QComboBox* m_configCombo;
    QPushButton* m_buildBtn;
    QPushButton* m_cleanBtn;
    QPushButton* m_rebuildBtn;
};
