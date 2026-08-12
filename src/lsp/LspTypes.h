#pragma once

#include <QString>
#include <QUrl>
#include <filesystem>
#include <vector>
#include <string>

namespace OpenIDE::Lsp {

inline QString pathToUri(const std::filesystem::path& path) {
    return QUrl::fromLocalFile(QString::fromStdString(path.string())).toString();
}

inline std::filesystem::path uriToPath(const QString& uriStr) {
    return QUrl(uriStr).toLocalFile().toStdString();
}

struct Position {
    int line = 0;      // 0-indexed
    int character = 0; // 0-indexed UTF-16 code units
};

struct Range {
    Position start;
    Position end;
};

struct Location {
    std::filesystem::path path;
    Range range;
};

struct HoverInfo {
    QString signature;
    QString documentation;
    Range range;
};

struct ParameterInfo {
    QString label;
    QString documentation;
};

struct SignatureInfo {
    QString label;
    QString documentation;
    std::vector<ParameterInfo> parameters;
    int activeParameter = 0;
};

struct DocumentSymbolInfo {
    QString name;
    QString detail;
    int kind = 0; // 5: Class, 6: Method, 12: Function, 13: Variable
    Range range;
    Range selectionRange;
    std::vector<DocumentSymbolInfo> children;
};

} // namespace OpenIDE::Lsp
