#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include "core/SymbolIndex.h"

class SearchEverywhereDialog : public QDialog {
    Q_OBJECT
public:
    explicit SearchEverywhereDialog(SymbolIndex* index, QWidget* parent = nullptr);

signals:
    void symbolSelected(const SymbolItem& item);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void onSearchTextChanged(const QString& text);
    void onItemActivated(QListWidgetItem* item);

private:
    void updateResults();

    SymbolIndex* m_index;
    QLineEdit* m_searchEdit;
    QListWidget* m_resultsList;
    QString m_activeFilter{"All"};
};
