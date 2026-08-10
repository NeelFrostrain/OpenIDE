#include "MainWindow.h"
#include "TitleBar.h"
#include "FileTreePanel.h"
#include "OutputPanel.h"
#include "ProblemsPanel.h"
#include "StructurePanel.h"
#include "GitPanel.h"
#include "TestRunnerPanel.h"
#include "NavigationBar.h"
#include "SearchEverywhereDialog.h"
#include "EditorWidget.h"
#include "CompletionPopup.h"
#include "CMakeBar.h"
#include "DebugToolbar.h"
#include "TerminalPanel.h"
#include "VariablesPanel.h"
#include "CallStackPanel.h"
#include "ActivityBar.h"
#include "StatusBar.h"
#include "refactor/RenameDialog.h"
#include "core/SymbolIndexer.h"
#include "vcs/GitManager.h"
#include "lsp/LspClient.h"
#include "dap/DapClient.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QDockWidget>
#include <QMenuBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QStandardPaths>
#include <QKeyEvent>
#include <QDateTime>
#include <QDirIterator>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    resize(1420, 900);

    m_gitManager = new GitManager(this);
    m_symbolIndexer = new SymbolIndexer(&m_symbolIndex, this);
    m_dapClient = new DapClient(this);

    m_centralHost = new QWidget(this);
    m_centralHost->setObjectName("centralHost");
    auto* outerLayout = new QVBoxLayout(m_centralHost);
    outerLayout->setContentsMargins(1, 1, 1, 1);
    outerLayout->setSpacing(0);

    m_titleBar = new TitleBar(this);
    connect(m_titleBar, &TitleBar::minimizeRequested, this, &QWidget::showMinimized);
    connect(m_titleBar, &TitleBar::maximizeRestoreRequested, this, [this] {
        isMaximized() ? showNormal() : showMaximized();
    });
    connect(m_titleBar, &TitleBar::closeRequested, this, &QWidget::close);
    outerLayout->addWidget(m_titleBar);

    auto* innerHost = new QWidget(m_centralHost);
    auto* innerHostLayout = new QHBoxLayout(innerHost);
    innerHostLayout->setContentsMargins(0, 0, 0, 0);
    innerHostLayout->setSpacing(0);

    m_activityBar = new ActivityBar(innerHost);
    innerHostLayout->addWidget(m_activityBar);

    auto* rightArea = new QWidget(innerHost);
    auto* rightAreaLayout = new QVBoxLayout(rightArea);
    rightAreaLayout->setContentsMargins(0, 0, 0, 0);
    rightAreaLayout->setSpacing(0);

    auto* inner = new QMainWindow(rightArea);
    inner->setWindowFlags(Qt::Widget);
    rightAreaLayout->addWidget(inner, 1);

    m_statusBar = new StatusBar(rightArea);
    rightAreaLayout->addWidget(m_statusBar);

    innerHostLayout->addWidget(rightArea, 1);
    outerLayout->addWidget(innerHost, 1);

    setCentralWidget(m_centralHost);

    // Menu bar
    auto* menuBar = new QMenuBar(inner);
    QMenu* fileMenu = menuBar->addMenu(tr("&File"));
    QAction* openFolderAction = fileMenu->addAction(tr("Open &Folder..."));
    connect(openFolderAction, &QAction::triggered, this, &MainWindow::openFolder);
    fileMenu->addSeparator();
    QAction* exitAction = fileMenu->addAction(tr("E&xit"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    QMenu* navMenu = menuBar->addMenu(tr("&Navigate"));
    QAction* searchAction = navMenu->addAction(tr("Search &Everywhere... (Ctrl+Shift+O)"));
    connect(searchAction, &QAction::triggered, this, &MainWindow::openSearchEverywhere);

    QMenu* refactorMenu = menuBar->addMenu(tr("&Refactor"));
    QAction* renameAction = refactorMenu->addAction(tr("&Rename Symbol... (F2)"));
    connect(renameAction, &QAction::triggered, this, &MainWindow::renameSymbolUnderCursor);

    inner->setMenuBar(menuBar);

    // Central area with Toolbars + NavigationBar + EditorTabs
    auto* centralContainer = new QWidget(inner);
    auto* centralLayout = new QVBoxLayout(centralContainer);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    auto* topBarsLayout = new QHBoxLayout();
    topBarsLayout->setContentsMargins(0, 0, 0, 0);
    topBarsLayout->setSpacing(0);

    m_cmakeBar = new CMakeBar(centralContainer);
    topBarsLayout->addWidget(m_cmakeBar);

    m_debugToolbar = new DebugToolbar(m_dapClient, centralContainer);
    topBarsLayout->addWidget(m_debugToolbar);

    centralLayout->addLayout(topBarsLayout);

    m_navigationBar = new NavigationBar(centralContainer);
    centralLayout->addWidget(m_navigationBar);

    m_editorTabs = new QTabWidget(centralContainer);
    m_editorTabs->setTabsClosable(true);
    m_editorTabs->setDocumentMode(true);
    connect(m_editorTabs, &QTabWidget::tabCloseRequested, this, &MainWindow::closeEditorTab);
    connect(m_editorTabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (auto* ed = qobject_cast<EditorWidget*>(m_editorTabs->widget(index))) {
            m_navigationBar->setPath(QDir::currentPath(), ed->filePath());
            m_structurePanel->setFileSymbols(ed->filePath(), m_symbolIndex.symbolsForFile(ed->filePath()));
        } else {
            m_navigationBar->setPath("", "");
            m_structurePanel->clearStructure();
        }
    });

    centralLayout->addWidget(m_editorTabs);
    inner->setCentralWidget(centralContainer);

    // Left Docks: FileTree & Structure
    m_fileTree = new FileTreePanel(inner);
    connect(m_fileTree, &FileTreePanel::fileActivated, this, &MainWindow::openFile);

    m_fileTreeDock = new QDockWidget(tr("Project"), inner);
    m_fileTreeDock->setWidget(m_fileTree);
    m_fileTreeDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    inner->addDockWidget(Qt::LeftDockWidgetArea, m_fileTreeDock);

    m_structurePanel = new StructurePanel(inner);
    m_structureDock = new QDockWidget(tr("Structure"), inner);
    m_structureDock->setWidget(m_structurePanel);
    m_structureDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    inner->tabifyDockWidget(m_fileTreeDock, m_structureDock);

    connect(m_structurePanel, &StructurePanel::symbolSelected, this, [this](const QString& filePath, int line, int col) {
        openFile(filePath);
        if (auto* ed = qobject_cast<EditorWidget*>(m_editorTabs->currentWidget())) {
            ed->gotoPosition(line, col);
        }
    });

    // ActivityBar actions
    m_activityBar->addActivity("📁", tr("Project Explorer"), [this] {
        m_fileTreeDock->isVisible() ? m_fileTreeDock->hide() : m_fileTreeDock->show();
    });
    m_activityBar->addActivity("🌐", tr("Structure"), [this] {
        m_structureDock->isVisible() ? m_structureDock->hide() : m_structureDock->show();
    });
    m_activityBar->addActivity("🌿", tr("Git VCS"), [this] {
        m_gitDock->isVisible() ? m_gitDock->hide() : m_gitDock->show();
    });
    m_activityBar->addActivity("💻", tr("Terminal"), [this] {
        m_terminalDock->isVisible() ? m_terminalDock->hide() : m_terminalDock->show();
    });
    m_activityBar->addActivity("⚠️", tr("Problems"), [this] {
        m_problemsDock->isVisible() ? m_problemsDock->hide() : m_problemsDock->show();
    });

    // Right Docks: Variables & CallStack
    m_variablesPanel = new VariablesPanel(inner);
    m_variablesDock = new QDockWidget(tr("Variables"), inner);
    m_variablesDock->setWidget(m_variablesPanel);
    m_variablesDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    inner->addDockWidget(Qt::RightDockWidgetArea, m_variablesDock);

    m_callStackPanel = new CallStackPanel(inner);
    m_callStackDock = new QDockWidget(tr("Call Stack"), inner);
    m_callStackDock->setWidget(m_callStackPanel);
    m_callStackDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    inner->tabifyDockWidget(m_variablesDock, m_callStackDock);

    // Bottom Docks: Output, Terminal, Problems, Git, TestRunner
    m_outputPanel = new OutputPanel(inner);
    m_outputDock = new QDockWidget(tr("Output"), inner);
    m_outputDock->setWidget(m_outputPanel);
    m_outputDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    inner->addDockWidget(Qt::BottomDockWidgetArea, m_outputDock);

    m_terminalPanel = new TerminalPanel(inner);
    m_terminalDock = new QDockWidget(tr("Terminal"), inner);
    m_terminalDock->setWidget(m_terminalPanel);
    m_terminalDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    inner->tabifyDockWidget(m_outputDock, m_terminalDock);

    m_problemsPanel = new ProblemsPanel(inner);
    m_problemsDock = new QDockWidget(tr("Problems"), inner);
    m_problemsDock->setWidget(m_problemsPanel);
    m_problemsDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    inner->tabifyDockWidget(m_outputDock, m_problemsDock);

    m_gitPanel = new GitPanel(m_gitManager, inner);
    m_gitDock = new QDockWidget(tr("Git"), inner);
    m_gitDock->setWidget(m_gitPanel);
    m_gitDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    inner->tabifyDockWidget(m_outputDock, m_gitDock);

    m_testPanel = new TestRunnerPanel(inner);
    m_testDock = new QDockWidget(tr("Test Runner"), inner);
    m_testDock->setWidget(m_testPanel);
    m_testDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    inner->tabifyDockWidget(m_outputDock, m_testDock);

    connect(m_problemsPanel, &ProblemsPanel::problemSelected, this, [this](const QString& filePath, int line, int col) {
        openFile(filePath);
        if (auto* editor = qobject_cast<EditorWidget*>(m_editorTabs->currentWidget())) {
            editor->gotoPosition(line, col);
        }
    });

    inner->resizeDocks({m_fileTreeDock, m_structureDock}, {260, 260}, Qt::Horizontal);
    inner->resizeDocks({m_outputDock, m_terminalDock, m_problemsDock, m_gitDock, m_testDock}, {180, 180, 180, 180, 180}, Qt::Vertical);

    m_outputPanel->appendLine(tr("MyIDE initialized with JetBrains Rider New UI visual design."));

    // Indexer signals
    connect(m_symbolIndexer, &SymbolIndexer::indexingStarted, this, [this] {
        m_outputPanel->appendLine(tr("Indexing project symbols in background..."));
    });

    connect(m_symbolIndexer, &SymbolIndexer::indexingFinished, this, [this](int totalSymbols) {
        m_outputPanel->appendLine(tr("Symbol indexing complete. %1 symbols indexed.").arg(totalSymbols));
        if (auto* ed = qobject_cast<EditorWidget*>(m_editorTabs->currentWidget())) {
            m_structurePanel->setFileSymbols(ed->filePath(), m_symbolIndex.symbolsForFile(ed->filePath()));
        }
    });

    // LSP Client Setup
    m_lspClient = new LspClient(this);

    connect(m_lspClient, &LspClient::serverStarted, this, [this] {
        m_statusBar->setLspStatus("clangd connected", true);
        m_outputPanel->appendLine(tr("[LSP] clangd process started."));
    });

    connect(m_lspClient, &LspClient::serverError, this, [this](const QString& err) {
        m_statusBar->setLspStatus("clangd error", false);
        m_outputPanel->appendLine(tr("[LSP ERROR] %1").arg(err));
    });

    connect(m_lspClient, &LspClient::initialized, this, [this] {
        m_outputPanel->appendLine(tr("[LSP] Handshake initialized. Registering open files..."));
        for (int i = 0; i < m_editorTabs->count(); ++i) {
            if (auto* ed = qobject_cast<EditorWidget*>(m_editorTabs->widget(i))) {
                QFile f(ed->filePath());
                if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    m_lspClient->didOpen(ed->filePath(), QString::fromUtf8(f.readAll()));
                }
            }
        }
    });

    connect(m_lspClient, &LspClient::diagnosticsReceived, this, [this](const QString& filePath, const QList<LspDiagnostic>& diagnostics) {
        m_problemsPanel->setDiagnostics(filePath, diagnostics);
        for (int i = 0; i < m_editorTabs->count(); ++i) {
            if (auto* ed = qobject_cast<EditorWidget*>(m_editorTabs->widget(i))) {
                if (ed->filePath() == filePath) {
                    ed->setDiagnostics(diagnostics);
                }
            }
        }
    });

    connect(m_lspClient, &LspClient::completionReady, this, [this](int reqId, const QList<LspCompletionItem>& items) {
        Q_UNUSED(reqId);
        if (auto* ed = qobject_cast<EditorWidget*>(m_editorTabs->currentWidget())) {
            if (ed->completionPopup()) {
                ed->completionPopup()->setCompletionItems(items);
            }
        }
    });

    applyTheme();

    QString currentDir = QDir::currentPath();
    m_fileTree->setRootPath(currentDir);
    m_symbolIndexer->startIndexing(currentDir);
    m_gitPanel->setRepositoryPath(currentDir);
    startLspServer(currentDir);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Shift) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        if (now - m_lastShiftTime < 400) {
            openSearchEverywhere();
            m_lastShiftTime = 0;
            return;
        }
        m_lastShiftTime = now;
    } else if (event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier) && event->key() == Qt::Key_O) {
        openSearchEverywhere();
        return;
    } else if (event->key() == Qt::Key_F2) {
        renameSymbolUnderCursor();
        return;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::renameSymbolUnderCursor() {
    auto* ed = qobject_cast<EditorWidget*>(m_editorTabs->currentWidget());
    if (!ed) return;

    QTextCursor cursor = ed->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    QString symbol = cursor.selectedText();

    RenameDialog dialog(symbol, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString newSym = dialog.newName();
        if (!newSym.isEmpty() && newSym != symbol) {
            m_outputPanel->appendLine(tr("Renaming '%1' to '%2'...").arg(symbol, newSym));
        }
    }
}

void MainWindow::openSearchEverywhere() {
    SearchEverywhereDialog dialog(&m_symbolIndex, this);
    connect(&dialog, &SearchEverywhereDialog::symbolSelected, this, [this](const SymbolItem& item) {
        if (!item.filePath.isEmpty()) {
            openFile(item.filePath);
            if (auto* ed = qobject_cast<EditorWidget*>(m_editorTabs->currentWidget())) {
                ed->gotoPosition(item.line, item.character);
            }
        }
    });
    dialog.exec();
}

void MainWindow::startLspServer(const QString& projectDir) {
    if (!m_lspClient) return;

    QString clangdCmd = QStandardPaths::findExecutable("clangd");
    if (clangdCmd.isEmpty()) {
        QString userLocal = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        QStringList candidates = {
            "C:/Program Files/LLVM/bin/clangd.exe",
            "D:/Applications/VS/VC/Tools/Llvm/x64/bin/clangd.exe",
            userLocal + "/Microsoft/WinGet/Packages/LLVM.clangd_Microsoft.Winget.Source_8wekyb3d8bbwe/clangd_22.1.6/bin/clangd.exe"
        };
        for (const auto& c : candidates) {
            if (QFile::exists(c)) {
                clangdCmd = c;
                break;
            }
        }
    }

    if (clangdCmd.isEmpty()) {
        QString appDataLocal = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        QDirIterator gitIt(appDataLocal + "/Microsoft/WinGet/Packages", {"clangd.exe"}, QDir::Files, QDirIterator::Subdirectories);
        if (gitIt.hasNext()) {
            clangdCmd = gitIt.next();
        }
    }

    if (clangdCmd.isEmpty()) {
        m_outputPanel->appendLine(tr("Note: clangd executable not found. Autocomplete & semantic LSP features will activate when clangd is detected."));
        return;
    }

    m_outputPanel->appendLine(tr("Starting clangd LSP server: %1").arg(clangdCmd));
    QStringList args = {"--background-index", "--compile-commands-dir=" + projectDir + "/build"};

    if (m_lspClient->startServer(clangdCmd, args, projectDir)) {
        m_lspClient->initialize(projectDir);
    }
}

void MainWindow::applyTheme() {
    QFile themeFile(":/themes/dark.qss");
    if (themeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(QString::fromUtf8(themeFile.readAll()));
    }
}

void MainWindow::openFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Folder"));
    if (dir.isEmpty()) return;
    m_fileTree->setRootPath(dir);
    m_titleBar->setTitle(tr("MyIDE — %1").arg(QFileInfo(dir).fileName()));
    m_outputPanel->appendLine(tr("Opened folder: %1").arg(dir));

    m_symbolIndexer->startIndexing(dir);
    m_gitPanel->setRepositoryPath(dir);
    startLspServer(dir);
}

void MainWindow::openFile(const QString& path) {
    for (int i = 0; i < m_editorTabs->count(); ++i) {
        if (auto* existing = qobject_cast<EditorWidget*>(m_editorTabs->widget(i))) {
            if (existing->filePath() == path) {
                m_editorTabs->setCurrentIndex(i);
                return;
            }
        }
    }

    auto* editor = new EditorWidget(m_editorTabs);
    editor->loadFile(path);

    connect(editor, &EditorWidget::cursorPositionChanged, this, [this, editor] {
        QTextCursor cursor = editor->textCursor();
        m_statusBar->setCursorPosition(cursor.blockNumber() + 1, cursor.columnNumber() + 1);
    });

    connect(editor, &EditorWidget::completionRequested, this, [this](const QString& filePath, int line, int col) {
        if (m_lspClient && m_lspClient->isRunning()) {
            m_lspClient->requestCompletion(filePath, line, col);
        }
    });

    int index = m_editorTabs->addTab(editor, QFileInfo(path).fileName());
    m_editorTabs->setCurrentIndex(index);

    m_navigationBar->setPath(QDir::currentPath(), path);
    m_structurePanel->setFileSymbols(path, m_symbolIndex.symbolsForFile(path));

    if (m_lspClient && m_lspClient->isRunning()) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_lspClient->didOpen(path, QString::fromUtf8(file.readAll()));
        }
    }
}

void MainWindow::closeEditorTab(int index) {
    QWidget* w = m_editorTabs->widget(index);
    m_editorTabs->removeTab(index);
    delete w;
}
