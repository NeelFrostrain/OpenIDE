#include "SymbolIndex.h"
#include <QFile>
#include <QJsonDocument>
#include <QFileInfo>
#include <QMutexLocker>
#include <algorithm>

QJsonObject SymbolItem::toJson() const {
    QJsonObject obj;
    obj["name"] = name;
    obj["kind"] = kind;
    obj["filePath"] = filePath;
    obj["line"] = line;
    obj["character"] = character;
    obj["containerName"] = containerName;
    return obj;
}

SymbolItem SymbolItem::fromJson(const QJsonObject& json) {
    SymbolItem item;
    item.name = json["name"].toString();
    item.kind = json["kind"].toString();
    item.filePath = json["filePath"].toString();
    item.line = json["line"].toInt();
    item.character = json["character"].toInt();
    item.containerName = json["containerName"].toString();
    return item;
}

void SymbolIndex::clear() {
    QMutexLocker locker(&m_mutex);
    m_symbols.clear();
}

void SymbolIndex::addSymbol(const SymbolItem& symbol) {
    QMutexLocker locker(&m_mutex);
    m_symbols.append(symbol);
}

void SymbolIndex::addSymbols(const QList<SymbolItem>& symbols) {
    QMutexLocker locker(&m_mutex);
    m_symbols.append(symbols);
}

void SymbolIndex::removeFileSymbols(const QString& filePath) {
    QMutexLocker locker(&m_mutex);
    auto it = std::remove_if(m_symbols.begin(), m_symbols.end(), [&](const SymbolItem& item) {
        return item.filePath == filePath;
    });
    m_symbols.erase(it, m_symbols.end());
}

QList<SymbolItem> SymbolIndex::symbolsForFile(const QString& filePath) const {
    QMutexLocker locker(&m_mutex);
    QList<SymbolItem> result;
    for (const auto& item : m_symbols) {
        if (item.filePath == filePath) {
            result.append(item);
        }
    }
    return result;
}

int SymbolIndex::calculateFuzzyScore(const QString& pattern, const QString& text) {
    if (pattern.isEmpty() || text.isEmpty()) return 0;

    QString p = pattern.toLower();
    QString t = text.toLower();

    // Exact prefix match gets highest score
    if (t.startsWith(p)) {
        return 1000 + (100 - t.length());
    }

    // Substring match
    int subIdx = t.indexOf(p);
    if (subIdx != -1) {
        return 500 + (100 - subIdx);
    }

    // Subsequence match
    int pIdx = 0;
    int score = 0;
    for (int tIdx = 0; tIdx < t.length() && pIdx < p.length(); ++tIdx) {
        if (t.at(tIdx) == p.at(pIdx)) {
            score += 10;
            pIdx++;
        }
    }

    if (pIdx == p.length()) {
        return score;
    }

    return -1; // No match
}

QList<SymbolItem> SymbolIndex::searchSymbols(const QString& query, int maxResults) const {
    QMutexLocker locker(&m_mutex);

    struct ScoredSymbol {
        SymbolItem item;
        int score;
    };

    QList<ScoredSymbol> scored;

    for (const auto& item : m_symbols) {
        int score = calculateFuzzyScore(query, item.name);
        if (score < 0) {
            // Also test matching against file basename
            int fileScore = calculateFuzzyScore(query, QFileInfo(item.filePath).fileName());
            if (fileScore > 0) {
                score = fileScore / 2;
            }
        }

        if (score > 0) {
            scored.append({item, score});
        }
    }

    std::sort(scored.begin(), scored.end(), [](const ScoredSymbol& a, const ScoredSymbol& b) {
        return a.score > b.score;
    });

    QList<SymbolItem> results;
    int limit = qMin(maxResults, static_cast<int>(scored.size()));
    for (int i = 0; i < limit; ++i) {
        results.append(scored[i].item);
    }

    return results;
}

bool SymbolIndex::saveToFile(const QString& cacheFilePath) const {
    QMutexLocker locker(&m_mutex);
    QJsonArray array;
    for (const auto& item : m_symbols) {
        array.append(item.toJson());
    }

    QJsonObject root;
    root["symbols"] = array;

    QFile file(cacheFilePath);
    if (!file.open(QIODevice::WriteOnly)) return false;

    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    return true;
}

bool SymbolIndex::loadFromFile(const QString& cacheFilePath) {
    QFile file(cacheFilePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) return false;

    QMutexLocker locker(&m_mutex);
    m_symbols.clear();

    QJsonArray array = doc.object()["symbols"].toArray();
    for (const auto& val : array) {
        if (val.isObject()) {
            m_symbols.append(SymbolItem::fromJson(val.toObject()));
        }
    }

    return true;
}
