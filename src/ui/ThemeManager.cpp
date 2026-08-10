#include "ui/ThemeManager.h"
#include <QFontDatabase>

namespace MyIDE::UI {

ThemeManager& ThemeManager::instance() {
    static ThemeManager s_instance;
    return s_instance;
}

ThemeManager::ThemeManager() {
}

QFont ThemeManager::editorFont() const {
    QStringList preferred = {"JetBrains Mono", "Cascadia Code", "Fira Code", "Consolas", "monospace"};
    QStringList available = QFontDatabase::families();

    QFont font;
    font.setPointSize(14);
    font.setFixedPitch(true);
    font.setStyleHint(QFont::Monospace);

    for (const auto& family : preferred) {
        if (available.contains(family, Qt::CaseInsensitive)) {
            font.setFamily(family);
            return font;
        }
    }
    font.setFamily("Consolas");
    return font;
}

QFont ThemeManager::uiFont() const {
    return QFont("Segoe UI", 9);
}

QPalette ThemeManager::darkPalette() const {
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, m_colors.windowBg);
    darkPalette.setColor(QPalette::WindowText, m_colors.text);
    darkPalette.setColor(QPalette::Base, m_colors.surfaceBg);
    darkPalette.setColor(QPalette::AlternateBase, m_colors.headerBg);
    darkPalette.setColor(QPalette::ToolTipBase, m_colors.surfaceBg);
    darkPalette.setColor(QPalette::ToolTipText, m_colors.text);
    darkPalette.setColor(QPalette::Text, m_colors.text);
    darkPalette.setColor(QPalette::Button, m_colors.headerBg);
    darkPalette.setColor(QPalette::ButtonText, m_colors.text);
    darkPalette.setColor(QPalette::BrightText, m_colors.error);
    darkPalette.setColor(QPalette::Highlight, m_colors.selection);
    darkPalette.setColor(QPalette::HighlightedText, QColor("#FFFFFF"));
    return darkPalette;
}

QString ThemeManager::globalStyleSheet() const {
    return R"(
        QMainWindow {
            background-color: #1E1E1E;
        }
        QSplitter::handle {
            background-color: #2D2D2D;
        }
        QSplitter::handle:hover {
            background-color: #007ACC;
        }
        QScrollBar:vertical {
            background: #1E1E1E;
            width: 10px;
            margin: 0px;
        }
        QScrollBar::handle:vertical {
            background: #3E3E42;
            min-height: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: #4F4F54;
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar:horizontal {
            background: #1E1E1E;
            height: 10px;
            margin: 0px;
        }
        QScrollBar::handle:horizontal {
            background: #3E3E42;
            min-width: 20px;
            border-radius: 4px;
        }
        QScrollBar::handle:horizontal:hover {
            background: #4F4F54;
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
        QMenuBar {
            background-color: #2D2D2D;
            color: #CCCCCC;
            border-bottom: 1px solid #3E3E42;
            font-size: 9pt;
        }
        QMenuBar::item {
            background: transparent;
            padding: 4px 8px;
        }
        QMenuBar::item:selected {
            background-color: #3C3C3C;
            color: #FFFFFF;
        }
        QMenu {
            background-color: #252526;
            color: #CCCCCC;
            border: 1px solid #3E3E42;
            padding: 4px;
            font-size: 9pt;
        }
        QMenu::item {
            padding: 5px 24px 5px 12px;
            border-radius: 3px;
        }
        QMenu::item:selected {
            background-color: #04395E;
            color: #FFFFFF;
        }
        QMenu::separator {
            height: 1px;
            background: #3E3E42;
            margin: 4px 0px;
        }
        QToolTip {
            background-color: #252526;
            color: #CCCCCC;
            border: 1px solid #3E3E42;
            padding: 4px;
        }
    )";
}

} // namespace MyIDE::UI
