#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QDockWidget>
#include "core/SymbolIndex.h"

class LspClient;
class ProblemsPanel;
class SymbolIndexer;
class NavigationBar;
class StructurePanel;
class GitManager;
class GitPanel;
class TestRunnerPanel;
class SearchEverywhereDialog;
class TitleBar;
class FileTreePanel;
class OutputPanel;
class DapClient;
class DebugToolbar;
class CMakeBar;
class TerminalPanel;
class VariablesPanel;
class CallStackPanel;
class ActivityBar;
class StatusBar;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void openFolder();
    void openFile(const QString& path);
    void closeEditorTab(int index);
    void startLspServer(const QString& projectDir);
    void openSearchEverywhere();
    void renameSymbolUnderCursor();

private:
    void buildMenus();
    void buildDockPanels();
    void applyTheme();

    TitleBar* m_titleBar = nullptr;
    QWidget* m_centralHost = nullptr;
    ActivityBar* m_activityBar = nullptr;
    StatusBar* m_statusBar = nullptr;
    CMakeBar* m_cmakeBar = nullptr;
    DebugToolbar* m_debugToolbar = nullptr;
    NavigationBar* m_navigationBar = nullptr;
    QTabWidget* m_editorTabs = nullptr;
    FileTreePanel* m_fileTree = nullptr;
    OutputPanel* m_outputPanel = nullptr;
    ProblemsPanel* m_problemsPanel = nullptr;
    StructurePanel* m_structurePanel = nullptr;
    GitPanel* m_gitPanel = nullptr;
    TestRunnerPanel* m_testPanel = nullptr;
    TerminalPanel* m_terminalPanel = nullptr;
    VariablesPanel* m_variablesPanel = nullptr;
    CallStackPanel* m_callStackPanel = nullptr;

    QDockWidget* m_fileTreeDock = nullptr;
    QDockWidget* m_outputDock = nullptr;
    QDockWidget* m_problemsDock = nullptr;
    QDockWidget* m_structureDock = nullptr;
    QDockWidget* m_gitDock = nullptr;
    QDockWidget* m_testDock = nullptr;
    QDockWidget* m_terminalDock = nullptr;
    QDockWidget* m_variablesDock = nullptr;
    QDockWidget* m_callStackDock = nullptr;

    LspClient* m_lspClient = nullptr;
    DapClient* m_dapClient = nullptr;
    SymbolIndex m_symbolIndex;
    SymbolIndexer* m_symbolIndexer = nullptr;
    GitManager* m_gitManager = nullptr;
    qint64 m_lastShiftTime{0};
};
