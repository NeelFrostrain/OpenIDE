#include "ui/MainWindow.h"
#include "editor/DocumentManager.h"
#include "project/ProjectManager.h"
#include "project/WorkspaceManager.h"
#include "core/Config.h"
#include "core/Logger.h"
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QSplitter>
#include <QHeaderView>
#include <QStatusBar>
#include <QApplication>
#include <QKeyEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>

namespace MyIDE::UI {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {

    setWindowTitle("MyIDE — Native C++ & Unreal Engine IDE");
    resize(1440, 900);

    m_lspClient = new Language::LspClient(this);
    m_completionService = new Language::CompletionService(m_lspClient, this);
    m_completionController = new Editor::CompletionController(m_completionService, this);
    m_searchDialog = new SearchEverywhereDialog(this);

    connect(&MyIDE::Core::Logger::instance(), &MyIDE::Core::Logger::logEmitted, this, &MainWindow::onLogEmitted);
    connect(m_lspClient, &Language::LspClient::diagnosticsPublished, this, &MainWindow::onDiagnosticsPublished);
    connect(m_lspClient, &Language::LspClient::serverReady, [this]() {
        m_lspStatusLabel->setText("clangd: Ready ✓");
    });

    connect(m_searchDialog, &SearchEverywhereDialog::fileSelected, [this](const std::filesystem::path& path, int line) {
        openFile(path);
        auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->currentWidget());
        if (editor && line > 0) editor->goToLine(line);
    });

    applyDarkTheme();

    // Central Container Layout
    QWidget* rootWidget = new QWidget(this);
    setCentralWidget(rootWidget);

    QHBoxLayout* rootLayout = new QHBoxLayout(rootWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    // Left Tool Bar
    m_leftToolBar = new LeftToolBar(this);
    rootLayout->addWidget(m_leftToolBar);

    // Right Workspace Area (Top Toolbar + Main Splitter)
    QWidget* workspaceWidget = new QWidget(this);
    QVBoxLayout* workspaceLayout = new QVBoxLayout(workspaceWidget);
    workspaceLayout->setContentsMargins(0, 0, 0, 0);
    workspaceLayout->setSpacing(0);

    createMenuBar();
    createTopToolBar();
    workspaceLayout->addWidget(m_topToolBar);

    QSplitter* mainSplitter = new QSplitter(Qt::Horizontal, workspaceWidget);
    workspaceLayout->addWidget(mainSplitter, 1);

    createSidebar();
    
    QSplitter* rightSplitter = new QSplitter(Qt::Vertical, mainSplitter);
    createEditorArea();
    createBottomPanels();

    QWidget* editorContainer = new QWidget(this);
    QVBoxLayout* editorLayout = new QVBoxLayout(editorContainer);
    editorLayout->setContentsMargins(0, 0, 0, 0);
    editorLayout->setSpacing(0);

    m_breadcrumbBar = new BreadcrumbBar(this);
    editorLayout->addWidget(m_breadcrumbBar);
    editorLayout->addWidget(m_editorTabs);

    rightSplitter->addWidget(editorContainer);
    rightSplitter->addWidget(m_bottomPanel);
    rightSplitter->setStretchFactor(0, 4);
    rightSplitter->setStretchFactor(1, 1);

    mainSplitter->addWidget(m_projectPanel);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 4);

    rootLayout->addWidget(workspaceWidget, 1);

    createStatusBar();

    connect(m_leftToolBar, &LeftToolBar::tabToggled, [this](ToolWindowTab tab, bool visible) {
        if (tab == ToolWindowTab::Project) {
            m_projectPanel->setVisible(visible);
        } else if (tab == ToolWindowTab::Run || tab == ToolWindowTab::Debug) {
            m_bottomPanel->setVisible(visible);
        }
    });

    MyIDE::Core::Logger::instance().info("UI", "MainWindow initialized with JetBrains Native IDE UI");

    // Open current working directory project workspace by default
    openFolder(std::filesystem::current_path());
}

MainWindow::~MainWindow() {
    delete m_fileModel;
}

void MainWindow::createMenuBar() {
    QMenuBar* menuBar = this->menuBar();

    // File Menu
    QMenu* fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction("&Open File...", QKeySequence::Open, this, &MainWindow::onOpenFileAction);
    fileMenu->addAction("Open &Folder...", this, &MainWindow::onOpenFolderAction);
    fileMenu->addSeparator();
    fileMenu->addAction("&Save", QKeySequence::Save, this, &MainWindow::onSaveAction);
    fileMenu->addAction("Save &All", QKeySequence("Ctrl+Shift+S"), this, &MainWindow::onSaveAllAction);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", QKeySequence::Quit, qApp, &QApplication::quit);

    // Edit Menu
    QMenu* editMenu = menuBar->addMenu("&Edit");
    editMenu->addAction("&Undo", QKeySequence::Undo, [this]() {
        if (auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->currentWidget())) editor->undo();
    });
    editMenu->addAction("&Redo", QKeySequence::Redo, [this]() {
        if (auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->currentWidget())) editor->redo();
    });

    // Navigate Menu
    QMenu* navMenu = menuBar->addMenu("&Navigate");
    navMenu->addAction("&Search Everywhere", QKeySequence("Ctrl+P"), this, &MainWindow::onSearchEverywhereAction);
    navMenu->addAction("Go to &Definition", QKeySequence(Qt::Key_F12), this, &MainWindow::onGoToDefinitionAction);
    navMenu->addAction("Find &References", QKeySequence("Shift+F12"), this, &MainWindow::onFindReferencesAction);
    navMenu->addAction("&Go to Line...", QKeySequence("Ctrl+G"), [this]() {
        auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->currentWidget());
        if (editor) editor->goToLine(10);
    });

    // View Menu
    QMenu* viewMenu = menuBar->addMenu("&View");
    viewMenu->addAction("Toggle &Sidebar", [this]() {
        m_projectPanel->setVisible(!m_projectPanel->isVisible());
    });
    viewMenu->addAction("Toggle &Output Panel", [this]() {
        m_bottomPanel->setVisible(!m_bottomPanel->isVisible());
    });
    viewMenu->addAction("Toggle &Focus Mode", QKeySequence("Ctrl+Shift+F11"), this, &MainWindow::toggleFocusMode);

    // Build Menu
    QMenu* buildMenu = menuBar->addMenu("&Build");
    buildMenu->addAction("&Build Project", QKeySequence("Ctrl+B"), []() {
        MyIDE::Core::Logger::instance().info("Build", "Starting project build...");
    });

    // Help Menu
    QMenu* helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction("&About MyIDE", [this]() {
        MyIDE::Core::Logger::instance().info("App", "MyIDE v0.1.0 — JetBrains-Inspired Native C++ IDE");
    });
}

void MainWindow::createTopToolBar() {
    m_topToolBar = new TopToolBar(this);
    connect(m_topToolBar, &TopToolBar::buildRequested, []() {
        MyIDE::Core::Logger::instance().info("Build", "Build requested from top toolbar");
    });
}

void MainWindow::createSidebar() {
    m_projectPanel = new QWidget(this);
    auto* layout = new QVBoxLayout(m_projectPanel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_projectHeader = new ToolWindowHeader("PROJECT", m_projectPanel);
    connect(m_projectHeader, &ToolWindowHeader::closeClicked, [this]() {
        m_projectPanel->hide();
    });

    m_fileModel = new QFileSystemModel(this);
    m_fileModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);

    m_treeView = new QTreeView(m_projectPanel);
    m_treeView->setModel(m_fileModel);
    m_treeView->setItemDelegate(new ProjectItemDelegate(m_treeView));
    m_treeView->header()->hide();
    m_treeView->hideColumn(1);
    m_treeView->hideColumn(2);
    m_treeView->hideColumn(3);

    m_treeView->setStyleSheet(R"(
        QTreeView {
            background-color: #252526;
            color: #CCCCCC;
            border: none;
            font-family: 'Segoe UI', sans-serif;
            font-size: 9pt;
        }
        QTreeView::item:hover {
            background-color: #2A2D2E;
        }
        QTreeView::item:selected {
            background-color: #37373D;
            color: #FFFFFF;
        }
    )");

    layout->addWidget(m_projectHeader);
    layout->addWidget(m_treeView);

    connect(m_treeView, &QTreeView::doubleClicked, [this](const QModelIndex& index) {
        if (!m_fileModel->isDir(index)) {
            QString path = m_fileModel->filePath(index);
            openFile(path.toStdString());
        }
    });
}

void MainWindow::createEditorArea() {
    m_editorTabs = new QTabWidget(this);
    m_editorTabs->setTabsClosable(true);
    m_editorTabs->setMovable(true);

    m_editorTabs->setStyleSheet(R"(
        QTabWidget::pane {
            border: none;
            background-color: #1E1E1E;
        }
        QTabBar::tab {
            background-color: #2D2D2D;
            color: #969696;
            padding: 6px 14px;
            border: none;
            margin-right: 1px;
            font-family: 'Segoe UI', sans-serif;
            font-size: 9pt;
        }
        QTabBar::tab:selected {
            background-color: #1E1E1E;
            color: #FFFFFF;
            border-top: 2px solid #007ACC;
        }
        QTabBar::tab:hover {
            background-color: #323232;
        }
    )");

    connect(m_editorTabs, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);
    connect(m_editorTabs, &QTabWidget::currentChanged, [this](int index) {
        auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->widget(index));
        if (editor) {
            m_breadcrumbBar->setPathAndSymbol(editor->filePath());
            m_completionController->attachEditor(editor);
        } else {
            m_completionController->detachEditor();
        }
    });
}

void MainWindow::createBottomPanels() {
    m_bottomPanel = new QWidget(this);
    auto* layout = new QVBoxLayout(m_bottomPanel);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_bottomHeader = new ToolWindowHeader("PROBLEMS / OUTPUT / TERMINAL", m_bottomPanel);
    connect(m_bottomHeader, &ToolWindowHeader::closeClicked, [this]() {
        m_bottomPanel->hide();
    });

    m_bottomTabs = new QTabWidget(m_bottomPanel);

    m_outputLog = new QPlainTextEdit(this);
    m_outputLog->setReadOnly(true);
    m_outputLog->setFont(QFont("Consolas", 10));
    m_outputLog->setStyleSheet("background-color: #1E1E1E; color: #CCCCCC; border: none;");

    m_problemsList = new QListWidget(this);
    m_problemsList->setStyleSheet("background-color: #1E1E1E; color: #F44336; border: none;");

    m_bottomTabs->addTab(m_outputLog, "Output");
    m_bottomTabs->addTab(m_problemsList, "Problems");

    m_bottomTabs->setStyleSheet(R"(
        QTabWidget::pane {
            border-top: 1px solid #333333;
            background-color: #1E1E1E;
        }
        QTabBar::tab {
            background-color: #252526;
            color: #969696;
            padding: 4px 10px;
            font-size: 9pt;
        }
        QTabBar::tab:selected {
            background-color: #1E1E1E;
            color: #FFFFFF;
        }
    )");

    layout->addWidget(m_bottomHeader);
    layout->addWidget(m_bottomTabs);
}

void MainWindow::createStatusBar() {
    QStatusBar* sb = statusBar();

    m_gitLabel = new QLabel("Git: main", this);
    m_statusLabel = new QLabel("Ready", this);
    m_cursorPosLabel = new QLabel("Ln 1, Col 1", this);
    m_lspStatusLabel = new QLabel("clangd: Offline", this);

    sb->addWidget(m_gitLabel);
    sb->addWidget(new QLabel("  |  ", this));
    sb->addWidget(m_statusLabel, 1);

    sb->addPermanentWidget(new QLabel("C++  |  UTF-8  |  LF  |  ", this));
    sb->addPermanentWidget(m_cursorPosLabel);
    sb->addPermanentWidget(new QLabel("  |  ", this));
    sb->addPermanentWidget(m_lspStatusLabel);

    sb->setStyleSheet("QStatusBar { background-color: #252526; border-top: 1px solid #2D2D2D; color: #999999; font-size: 9pt; }");
}

void MainWindow::applyDarkTheme() {
    qApp->setPalette(ThemeManager::instance().darkPalette());
    qApp->setStyleSheet(ThemeManager::instance().globalStyleSheet());
}

void MainWindow::openFile(const std::filesystem::path& path) {
    auto* doc = Editor::DocumentManager::instance().openDocument(path);
    if (!doc) return;

    // Check if tab already exists
    for (int i = 0; i < m_editorTabs->count(); ++i) {
        auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->widget(i));
        if (editor && editor->documentModel() == doc) {
            m_editorTabs->setCurrentIndex(i);
            m_breadcrumbBar->setPathAndSymbol(path);
            return;
        }
    }

    auto* editor = new Editor::EditorWidget(doc, this);
    int tabIndex = m_editorTabs->addTab(editor, doc->fileName());
    m_editorTabs->setCurrentIndex(tabIndex);
    m_breadcrumbBar->setPathAndSymbol(path);
    m_completionController->attachEditor(editor);

    connect(editor, &Editor::EditorWidget::completionRequested, this, &MainWindow::onCompletionRequested);
    connect(editor, &QPlainTextEdit::cursorPositionChanged, [this, editor]() {
        QTextCursor c = editor->textCursor();
        m_cursorPosLabel->setText(QString("Ln %1, Col %2").arg(c.blockNumber() + 1).arg(c.positionInBlock() + 1));
    });

    m_lspClient->didOpen(path, doc->content());

    connect(doc, &Editor::Document::contentChanged, [this, path, doc](const QString& text, int ver) {
        m_lspClient->didChange(path, text, ver);
    });
}

void MainWindow::openFolder(const std::filesystem::path& path) {
    if (!Project::ProjectManager::instance().openProject(path)) return;

    QString rootPath = QString::fromStdString(path.string());
    m_fileModel->setRootPath(rootPath);
    m_treeView->setRootIndex(m_fileModel->index(rootPath));

    m_searchDialog->setFiles(Project::ProjectManager::instance().sourceFiles());

    // Launch clangd LSP server for this workspace
    QString clangdPath = MyIDE::Core::Config::instance().clangdExecutable();
    m_lspClient->start(clangdPath, path);
    m_lspStatusLabel->setText("clangd: Starting...");

    // Restore workspace state from .ide/workspace/workspace.json
    auto state = Project::WorkspaceManager::instance().loadWorkspace();
    for (const auto& fileStr : state.openFiles) {
        if (std::filesystem::exists(fileStr)) {
            openFile(fileStr);
            auto it = state.cursorPositions.find(fileStr);
            if (it != state.cursorPositions.end()) {
                auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->currentWidget());
                if (editor) editor->goToLine(it->second.line, it->second.column);
            }
        }
    }
    if (!state.activeFile.empty() && std::filesystem::exists(state.activeFile)) {
        openFile(state.activeFile);
    }
}

void MainWindow::onOpenFileAction() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "C++ Files (*.cpp *.h *.hpp *.c *.cs);;All Files (*)");
    if (!fileName.isEmpty()) {
        openFile(fileName.toStdString());
    }
}

void MainWindow::onOpenFolderAction() {
    QString folder = QFileDialog::getExistingDirectory(this, "Open Project Directory");
    if (!folder.isEmpty()) {
        openFolder(folder.toStdString());
    }
}

void MainWindow::onSaveAction() {
    auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->currentWidget());
    if (editor && editor->documentModel()) {
        editor->documentModel()->save();
        m_lspClient->didSave(editor->filePath());
    }
}

void MainWindow::onSaveAllAction() {
    Editor::DocumentManager::instance().saveAll();
}

void MainWindow::onTabCloseRequested(int index) {
    auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->widget(index));
    if (editor && editor->documentModel()) {
        Editor::DocumentManager::instance().closeDocument(editor->documentModel()->id());
    }
    m_editorTabs->removeTab(index);
}

void MainWindow::onSearchEverywhereAction() {
    m_searchDialog->exec();
}

void MainWindow::onGoToDefinitionAction() {
    auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->currentWidget());
    if (!editor) return;

    QTextCursor cursor = editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.positionInBlock();

    m_lspClient->requestDefinition(editor->filePath(), line, col, [this](const std::vector<Language::LocationResult>& results) {
        if (!results.empty()) {
            const auto& target = results[0];
            openFile(target.path);
            auto* targetEditor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->currentWidget());
            if (targetEditor) {
                targetEditor->goToLine(target.line, target.column);
            }
        }
    });
}

void MainWindow::onFindReferencesAction() {
    auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->currentWidget());
    if (!editor) return;

    QTextCursor cursor = editor->textCursor();
    int line = cursor.blockNumber() + 1;
    int col = cursor.positionInBlock();

    m_lspClient->requestReferences(editor->filePath(), line, col, [this](const std::vector<Language::LocationResult>& results) {
        m_problemsList->clear();
        m_bottomTabs->setCurrentIndex(1); // Show problems panel for reference results
        for (const auto& r : results) {
            m_problemsList->addItem(QString("Ref: %1 [%2:%3]").arg(QString::fromStdString(r.path.filename().string())).arg(r.line).arg(r.column));
        }
    });
}

void MainWindow::toggleFocusMode() {
    m_focusMode = !m_focusMode;
    m_leftToolBar->setVisible(!m_focusMode);
    m_topToolBar->setVisible(!m_focusMode);
    m_projectPanel->setVisible(!m_focusMode);
    m_bottomPanel->setVisible(!m_focusMode);
    statusBar()->setVisible(!m_focusMode);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    QMainWindow::keyPressEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    Project::WorkspaceState state;
    state.version = 1;

    auto* currentEditor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->currentWidget());
    if (currentEditor) {
        state.activeFile = currentEditor->filePath().string();
    }

    for (int i = 0; i < m_editorTabs->count(); ++i) {
        auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->widget(i));
        if (editor) {
            std::string pathStr = editor->filePath().string();
            state.openFiles.push_back(pathStr);
            QTextCursor c = editor->textCursor();
            state.cursorPositions[pathStr] = {c.blockNumber() + 1, c.positionInBlock() + 1};
        }
    }

    if (m_completionController) {
        m_completionController->cancelSession(Editor::CancelReason::WindowMinimized);
    }

    Project::WorkspaceManager::instance().saveWorkspace(state);
    QMainWindow::closeEvent(event);
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {
        if (isMinimized() && m_completionController) {
            m_completionController->cancelSession(Editor::CancelReason::WindowMinimized);
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::moveEvent(QMoveEvent* event) {
    if (m_completionController && m_completionController->isSessionActive()) {
        m_completionController->cancelSession(Editor::CancelReason::CursorMoved);
    }
    QMainWindow::moveEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    if (m_completionController && m_completionController->isSessionActive()) {
        m_completionController->cancelSession(Editor::CancelReason::CursorMoved);
    }
    QMainWindow::resizeEvent(event);
}

void MainWindow::onCompletionRequested(const QString& prefix, int line, int col) {
    Q_UNUSED(prefix);
    Q_UNUSED(line);
    Q_UNUSED(col);
    if (m_completionController) {
        m_completionController->triggerCompletion(false);
    }
}

void MainWindow::onDiagnosticsPublished(const std::filesystem::path& path, const std::vector<Editor::Diagnostic>& diagnostics) {
    m_problemsList->clear();
    for (const auto& d : diagnostics) {
        m_problemsList->addItem(QString("%1 [%2:%3] %4").arg(QString::fromStdString(path.filename().string())).arg(d.startLine).arg(d.startColumn).arg(d.message));
    }

    for (int i = 0; i < m_editorTabs->count(); ++i) {
        auto* editor = qobject_cast<Editor::EditorWidget*>(m_editorTabs->widget(i));
        if (editor && editor->filePath() == path) {
            editor->setDiagnostics(diagnostics);
        }
    }
}

void MainWindow::onLogEmitted(MyIDE::Core::LogLevel level, const QString& category, const QString& message, const QString& formattedMessage) {
    Q_UNUSED(level);
    Q_UNUSED(category);
    Q_UNUSED(message);
    m_outputLog->appendPlainText(formattedMessage);
}

} // namespace MyIDE::UI
