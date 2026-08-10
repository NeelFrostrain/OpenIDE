#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

class RenameDialog : public QDialog {
    Q_OBJECT
public:
    explicit RenameDialog(const QString& oldName, QWidget* parent = nullptr);

    QString newName() const;

private:
    QLineEdit* m_nameEdit;
};
