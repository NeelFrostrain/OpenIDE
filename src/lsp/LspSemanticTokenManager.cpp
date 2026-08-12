#include "lsp/LspSemanticTokenManager.h"
#include "ui/ThemeManager.h"

namespace OpenIDE::Lsp {

LspSemanticTokenManager::LspSemanticTokenManager(QObject* parent)
    : QObject(parent) {
}

void LspSemanticTokenManager::setLegend(const std::vector<std::string>& tokenTypes, const std::vector<std::string>& tokenModifiers) {
    m_tokenTypes = tokenTypes;
    m_tokenModifiers = tokenModifiers;
}

std::vector<SemanticToken> LspSemanticTokenManager::decodeTokens(const std::vector<uint32_t>& data) const {
    std::vector<SemanticToken> result;
    if (data.size() % 5 != 0) return result;

    int currentLine = 0;
    int currentStartChar = 0;

    for (size_t i = 0; i < data.size(); i += 5) {
        uint32_t deltaLine = data[i];
        uint32_t deltaStartChar = data[i + 1];
        uint32_t length = data[i + 2];
        uint32_t tokenType = data[i + 3];
        uint32_t tokenModifiers = data[i + 4];

        currentLine += deltaLine;
        if (deltaLine > 0) {
            currentStartChar = deltaStartChar;
        } else {
            currentStartChar += deltaStartChar;
        }

        SemanticToken tok;
        tok.line = currentLine;
        tok.startChar = currentStartChar;
        tok.length = length;
        tok.tokenType = tokenType;
        tok.tokenModifiers = tokenModifiers;

        if (tokenType < m_tokenTypes.size()) {
            tok.typeName = m_tokenTypes[tokenType];
        }

        result.push_back(tok);
    }

    return result;
}

QColor LspSemanticTokenManager::colorForTokenType(const std::string& typeName) const {
    const auto& colors = UI::ThemeManager::instance().colors();

    if (typeName == "class" || typeName == "struct" || typeName == "enum" || typeName == "type" || typeName == "interface") {
        return colors.type; // Teal #4EC9B0
    } else if (typeName == "function" || typeName == "method") {
        return colors.function; // Yellow #DCDCAA
    } else if (typeName == "parameter" || typeName == "property" || typeName == "field") {
        return colors.parameter; // Light Blue #9CDCFE
    } else if (typeName == "namespace") {
        return colors.nameSpace; // Blue #569CD6
    } else if (typeName == "keyword" || typeName == "macro") {
        return colors.keyword; // Purple #C586C0
    }

    return colors.text; // #D4D4D4
}

} // namespace OpenIDE::Lsp
