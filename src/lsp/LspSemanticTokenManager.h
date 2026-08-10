#pragma once

#include "lsp/LspTypes.h"
#include <QObject>
#include <QColor>
#include <vector>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace MyIDE::Lsp {

struct SemanticToken {
    int line = 0;
    int startChar = 0;
    int length = 0;
    int tokenType = 0;
    int tokenModifiers = 0;
    std::string typeName;
};

class LspSemanticTokenManager : public QObject {
    Q_OBJECT

public:
    explicit LspSemanticTokenManager(QObject* parent = nullptr);

    void setLegend(const std::vector<std::string>& tokenTypes, const std::vector<std::string>& tokenModifiers);
    std::vector<SemanticToken> decodeTokens(const std::vector<uint32_t>& data) const;

    QColor colorForTokenType(const std::string& typeName) const;

private:
    std::vector<std::string> m_tokenTypes;
    std::vector<std::string> m_tokenModifiers;
};

} // namespace MyIDE::Lsp
