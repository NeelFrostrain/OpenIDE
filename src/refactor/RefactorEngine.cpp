#include "RefactorEngine.h"

ExtractMethodResult RefactorEngine::extractMethod(const QString& selectedText, const QString& functionName) {
    ExtractMethodResult res;
    if (selectedText.trimmed().isEmpty() || functionName.trimmed().isEmpty()) {
        return res;
    }

    QString fnName = functionName.trimmed();

    // Construct helper function
    res.extractedFunctionCode = QString("void %1() {\n    %2\n}\n\n")
                                    .arg(fnName, selectedText.trimmed().replace("\n", "\n    "));

    // Replace selection with function call
    res.replacementCallText = QString("%1();").arg(fnName);
    res.success = true;

    return res;
}

QString RefactorEngine::expandSnippet(const QString& abbreviation) {
    if (abbreviation == "forr") {
        return "for (auto& item : container) {\n    \n}";
    } else if (abbreviation == "fori") {
        return "for (int i = 0; i < n; ++i) {\n    \n}";
    } else if (abbreviation == "cout") {
        return "std::cout <<  << std::endl;";
    } else if (abbreviation == "ifn") {
        return "if (!ptr) {\n    return;\n}";
    }
    return QString();
}
