#pragma once

#include <QThread>
#include <QString>
#include <QList>
#include "SymbolIndex.h"

class SymbolIndexer : public QThread {
    Q_OBJECT
public:
    explicit SymbolIndexer(SymbolIndex* index, QObject* parent = nullptr);
    ~SymbolIndexer() override;

    void startIndexing(const QString& projectPath);

signals:
    void indexingStarted();
    void indexingProgress(int current, int total);
    void indexingFinished(int totalSymbols);

protected:
    void run() override;

private:
    SymbolIndex* m_index;
    QString m_projectPath;

    QList<SymbolItem> parseFileSymbols(const QString& filePath);
};
