#pragma once

#include "editor/EditorWidget.h"
#include "editor/CompletionController.h"
#include "language/LspClient.h"
#include "language/CompletionService.h"
#include "core/Logger.h"
#include "ui/ThemeManager.h"
#include "ui/LeftToolBar.h"
#include "ui/TopToolBar.h"
#include "ui/ToolWindowHeader.h"
#include "ui/ProjectItemDelegate.h"
#include "ui/BreadcrumbBar.h"
#include "ui/SearchEverywhereDialog.h"
#include <QMainWindow>
#include <QTabWidget>
#include <QTreeView>
#include <QFileSystemModel>
#include <QPlainTextEdit>
#include <QLabel>
#include <QListWidget>

namespace MyIDE::UI {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void openFile(const std::filesystem::path& path);
    void openFolder(const std::filesystem::path& path);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void moveEvent(QMoveEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onOpenFileAction();
    void onOpenFolderAction();
    void onSaveAction();
    void onSaveAllAction();
    void onTabCloseRequested(int index);
    void onSearchEverywhereAction();
    void onGoToDefinitionAction();
    void onFindReferencesAction();
    void toggleFocusMode();

    void onCompletionRequested(const QString& prefix, int line, int col);
    void onDiagnosticsPublished(const std::filesystem::path& path, const std::vector<Editor::Diagnostic>& diagnostics);
    void onLogEmitted(MyIDE::Core::LogLevel level, const QString& category, const QString& message, const QString& formattedMessage);

private:
    void createMenuBar();
    void createTopToolBar();
    void createSidebar();
    void createEditorArea();
    void createBottomPanels();
    void createStatusBar();
    void applyDarkTheme();

    LeftToolBar* m_leftToolBar = nullptr;
    TopToolBar* m_topToolBar = nullptr;

    QWidget* m_projectPanel = nullptr;
    ToolWindowHeader* m_projectHeader = nullptr;
    QFileSystemModel* m_fileModel = nullptr;
    QTreeView* m_treeView = nullptr;

    QTabWidget* m_editorTabs = nullptr;
    BreadcrumbBar* m_breadcrumbBar = nullptr;

    QWidget* m_bottomPanel = nullptr;
    ToolWindowHeader* m_bottomHeader = nullptr;
    QTabWidget* m_bottomTabs = nullptr;
    QPlainTextEdit* m_outputLog = nullptr;
    QListWidget* m_problemsList = nullptr;

    QLabel* m_statusLabel = nullptr;
    QLabel* m_gitLabel = nullptr;
    QLabel* m_cursorPosLabel = nullptr;
    QLabel* m_lspStatusLabel = nullptr;

    Language::LspClient* m_lspClient = nullptr;
    Language::CompletionService* m_completionService = nullptr;
    Editor::CompletionController* m_completionController = nullptr;
    SearchEverywhereDialog* m_searchDialog = nullptr;

    bool m_focusMode = false;
};

} // namespace MyIDE::UI
