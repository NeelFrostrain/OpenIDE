#include "SymbolIndexer.h"
#include <QDirIterator>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

extern "C" {
#include <tree_sitter/api.h>
const TSLanguage *tree_sitter_cpp(void);
}

SymbolIndexer::SymbolIndexer(SymbolIndex* index, QObject* parent)
    : QThread(parent), m_index(index) {}

SymbolIndexer::~SymbolIndexer() {
    requestInterruption();
    wait();
}

void SymbolIndexer::startIndexing(const QString& projectPath) {
    if (isRunning()) {
        requestInterruption();
        wait();
    }
    m_projectPath = projectPath;
    start();
}

void SymbolIndexer::run() {
    if (!m_index || m_projectPath.isEmpty()) return;

    emit indexingStarted();

    QStringList files;
    QDirIterator it(m_projectPath, {"*.cpp", "*.h", "*.hpp", "*.c", "*.cc", "*.cxx"},
                    QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        if (!path.contains("/build/") && !path.contains("/.git/")) {
            files.append(path);
        }
    }

    int total = files.size();
    int current = 0;
    int totalSymbols = 0;

    m_index->clear();

    for (const auto& filePath : files) {
        if (isInterruptionRequested()) return;

        // Also index the file itself as a "file" symbol
        SymbolItem fileItem;
        fileItem.name = QFileInfo(filePath).fileName();
        fileItem.kind = "file";
        fileItem.filePath = filePath;
        fileItem.line = 0;
        fileItem.character = 0;
        m_index->addSymbol(fileItem);
        totalSymbols++;

        QList<SymbolItem> symbols = parseFileSymbols(filePath);
        m_index->addSymbols(symbols);
        totalSymbols += symbols.size();

        current++;
        emit indexingProgress(current, total);
    }

    emit indexingFinished(totalSymbols);
}

QList<SymbolItem> SymbolIndexer::parseFileSymbols(const QString& filePath) {
    QList<SymbolItem> symbols;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return symbols;

    QByteArray bytes = file.readAll();
    TSParser* parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_cpp());

    TSTree* tree = ts_parser_parse_string(parser, nullptr, bytes.constData(), bytes.size());
    if (!tree) {
        ts_parser_delete(parser);
        return symbols;
    }

    TSNode root = ts_tree_root_node(tree);
    if (ts_node_is_null(root)) {
        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return symbols;
    }

    uint32_t childCount = ts_node_child_count(root);

    for (uint32_t i = 0; i < childCount; ++i) {
        TSNode child = ts_node_child(root, i);
        if (ts_node_is_null(child)) continue;
        const char* type = ts_node_type(child);

        if (!type) continue;

        QString kind;
        if (std::strcmp(type, "function_definition") == 0) kind = "function";
        else if (std::strcmp(type, "class_specifier") == 0) kind = "class";
        else if (std::strcmp(type, "struct_specifier") == 0) kind = "struct";
        else if (std::strcmp(type, "enum_specifier") == 0) kind = "enum";
        else if (std::strncmp(type, "preproc_", 8) == 0) kind = "macro";

        if (!kind.isEmpty()) {
            uint32_t startByte = ts_node_start_byte(child);
            TSPoint startPoint = ts_node_start_point(child);

            // Extract symbol name by walking child nodes for identifier
            QString name;
            uint32_t subCount = ts_node_child_count(child);
            for (uint32_t j = 0; j < subCount; ++j) {
                TSNode sub = ts_node_child(child, j);
                const char* subType = ts_node_type(sub);
                if (subType && (std::strcmp(subType, "identifier") == 0 ||
                                std::strcmp(subType, "type_identifier") == 0 ||
                                std::strcmp(subType, "function_declarator") == 0)) {
                    uint32_t sByte = ts_node_start_byte(sub);
                    uint32_t eByte = ts_node_end_byte(sub);
                    name = QString::fromUtf8(bytes.mid(sByte, eByte - sByte));
                    break;
                }
            }

            if (!name.isEmpty()) {
                SymbolItem item;
                item.name = name;
                item.kind = kind;
                item.filePath = filePath;
                item.line = startPoint.row;
                item.character = startPoint.column;
                symbols.append(item);
            }
        }
    }

    ts_tree_delete(tree);
    ts_parser_delete(parser);

    return symbols;
}
