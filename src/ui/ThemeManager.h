#pragma once

#include <QColor>
#include <QFont>
#include <QString>
#include <QPalette>

namespace OpenIDE::UI {

struct ThemeColors {
    QColor windowBg       = QColor("#1E1E1E");
    QColor surfaceBg      = QColor("#252526");
    QColor surfaceDark    = QColor("#1B1B1C");
    QColor headerBg       = QColor("#2D2D2D");
    QColor border         = QColor("#3E3E42");
    QColor text           = QColor("#D4D4D4");
    QColor textMuted      = QColor("#808080");
    QColor accent         = QColor("#007ACC");
    QColor selection      = QColor("#04395E");
    QColor hover          = QColor("#2A2D2E");
    QColor error          = QColor("#F44336");
    QColor warning        = QColor("#FF9800");
    QColor success        = QColor("#4CAF50");

    // Syntax colors
    QColor keyword        = QColor("#C586C0");
    QColor type           = QColor("#4EC9B0");
    QColor function       = QColor("#DCDCAA");
    QColor string         = QColor("#CE9178");
    QColor number         = QColor("#B5CEA8");
    QColor comment        = QColor("#6A9955");
    QColor macro          = QColor("#C586C0");
    QColor parameter      = QColor("#9CDCFE");
    QColor field          = QColor("#9CDCFE");
    QColor nameSpace      = QColor("#569CD6");
    QColor op             = QColor("#D4D4D4");
};

class ThemeManager {
public:
    static ThemeManager& instance();

    const ThemeColors& colors() const { return m_colors; }
    QString globalStyleSheet() const;
    QPalette darkPalette() const;

    QFont editorFont() const;
    QFont uiFont() const;

private:
    ThemeManager();
    ThemeColors m_colors;
};

} // namespace OpenIDE::UI
