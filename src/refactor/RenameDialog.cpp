#include "RenameDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

RenameDialog::RenameDialog(const QString& oldName, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Rename Symbol"));
    resize(380, 140);
    setStyleSheet("QDialog { background-color: #1e1f22; color: #dfe1e5; }"
                  "QLineEdit { background-color: #2b2d30; color: #dfe1e5; font-size: 13px; padding: 6px; border: 1px solid #4e5157; border-radius: 4px; }"
                  "QPushButton { background-color: #2e436e; color: #ffffff; padding: 6px 14px; border-radius: 4px; font-weight: bold; }");

    auto* mainLayout = new QVBoxLayout(this);

    auto* lbl = new QLabel(tr("Enter new name for symbol '%1':").arg(oldName), this);
    mainLayout->addWidget(lbl);

    m_nameEdit = new QLineEdit(oldName, this);
    m_nameEdit->selectAll();
    mainLayout->addWidget(m_nameEdit);

    auto* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();

    auto* okBtn = new QPushButton(tr("Rename"), this);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(okBtn);

    auto* cancelBtn = new QPushButton(tr("Cancel"), this);
    cancelBtn->setStyleSheet("background-color: #393b40; color: #dfe1e5;");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    btnLayout->addWidget(cancelBtn);

    mainLayout->addLayout(btnLayout);
}

QString RenameDialog::newName() const {
    return m_nameEdit->text().trimmed();
}
