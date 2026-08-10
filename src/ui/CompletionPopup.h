#pragma once

#include <QListWidget>
#include "lsp/LspTypes.h"

class CompletionPopup : public QListWidget {
    Q_OBJECT
public:
    explicit CompletionPopup(QWidget* parent = nullptr);

    void setCompletionItems(const QList<LspCompletionItem>& items);
    void filter(const QString& prefix);

signals:
    void itemSelected(const QString& text);

public:
    void keyPressEvent(QKeyEvent* event) override;

private:
    QList<LspCompletionItem> m_allItems;
};
