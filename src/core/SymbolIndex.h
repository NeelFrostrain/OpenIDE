#pragma once

#include <QString>
#include <QList>
#include <QMutex>
#include <QJsonObject>
#include <QJsonArray>

struct SymbolItem {
    QString name;
    QString kind; // "class", "function", "struct", "enum", "variable", "file", "action"
    QString filePath;
    int line{0};
    int character{0};
    QString containerName;

    QJsonObject toJson() const;
    static SymbolItem fromJson(const QJsonObject& json);
};

class SymbolIndex {
public:
    SymbolIndex() = default;

    void clear();
    void addSymbol(const SymbolItem& symbol);
    void addSymbols(const QList<SymbolItem>& symbols);
    void removeFileSymbols(const QString& filePath);

    QList<SymbolItem> searchSymbols(const QString& query, int maxResults = 50) const;
    QList<SymbolItem> symbolsForFile(const QString& filePath) const;

    bool saveToFile(const QString& cacheFilePath) const;
    bool loadFromFile(const QString& cacheFilePath);

private:
    mutable QMutex m_mutex;
    QList<SymbolItem> m_symbols;
    
    static int calculateFuzzyScore(const QString& pattern, const QString& text);
};
