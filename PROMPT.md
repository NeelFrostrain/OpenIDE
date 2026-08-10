# Project Brief: "MyIDE" — a native, modern, LSP/DAP-powered IDE

Use this document as the standing spec/prompt when working on this project (in Claude Code,
another AI agent, or by hand). Paste it at the start of a session so the agent has full context
without you re-explaining the architecture every time.

## 1. Vision

Build a native desktop IDE (think "indie CLion/Rider") that supports editing, building, and
debugging C++ first, with a clean path to add other languages (Rust, Python, Go, TypeScript)
by swapping language servers. Native performance, modern flat UI (JetBrains-Darcula-like dark
theme by default), no Electron.

Non-goals (at least for v1): solution/project-file formats like .sln, NuGet/package-manager
integration, remote/container debugging, plugin marketplace, Unity-specific tooling. These can
come later once the core loop (edit → build → debug) is solid.

## 2. Tech stack (locked in)

- **Language:** C++20
- **UI framework:** Qt 6, Widgets module (not QML)
- **Build system:** CMake ≥ 3.21 + Ninja
- **Editor widget:** start with `QPlainTextEdit` + custom line-number gutter (prototype);
  migrate to QScintilla (via QScintilla2) once the prototype's editing UX is validated
- **Syntax highlighting:** tree-sitter (incremental, real AST) — not Scintilla's built-in lexers
- **LSP client:** hand-rolled JSON-RPC 2.0 over stdio (`QProcess`), talks to `clangd` first
- **DAP client:** hand-rolled JSON-RPC 2.0 over stdio, talks to `lldb-dap` first
- **Docking:** Qt's built-in `QDockWidget` for the prototype; migrate to Qt Advanced Docking
  System (KDAB, LGPL) once we need floating/tabbed dock groups
- **JSON:** `QJsonDocument`/`QJsonObject` (already in Qt, no extra dependency needed yet)
- **Fonts:** bundle Inter (UI) and JetBrains Mono (editor) as application fonts
- **Icons:** Lucide icon set (SVG, ISC license), tinted at runtime to match theme

## 3. Architecture (target — prototype implements a slice of this)

```
src/
  core/
    Workspace          // "open folder" model, recursive file watch
    Project            // parses compile_commands.json, exposes per-file compile flags
  editor/
    EditorWidget        // one per open file: text buffer + gutter + diagnostics squiggles
    TreeSitterHighlighter// incremental syntax highlighting
  lsp/
    LspClient            // JSON-RPC framing, request/response/notification dispatch
    LspTypes              // Position, Range, Diagnostic, CompletionItem, etc.
  dap/
    DapClient             // JSON-RPC framing for DAP
    DapSession            // breakpoint set, stack frames, scopes/variables, run state
  build/
    BuildRunner           // shells to cmake/ninja, streams stdout/stderr, parses diagnostics
  ui/
    TitleBar              // custom frameless-window title bar
    MainWindow            // top-level window, dock layout, menu/toolbar, theme loader
    FileTreePanel         // QFileSystemModel-backed project tree
    OutputPanel           // build output / console
    DebugPanel            // call stack, locals, watches, breakpoints list
  main.cpp
resources/
  themes/dark.qss, themes/light.qss   // generated or hand-written QSS
  fonts/
  icons/
```

## 4. Milestones (in order — do not skip ahead)

1. **Shell (this prototype covers this):** frameless window + custom title bar, dark theme via
   QSS, dockable file tree / editor / output panels, multi-tab plain-text editing, open-folder
   flow. No language intelligence yet.
2. **Syntax highlighting:** wire tree-sitter into the editor for C++ (and one more language) to
   prove the abstraction.
3. **LSP integration (clangd):** spawn on project open (using `compile_commands.json` location),
   wire `initialize`/`didOpen`/`didChange`, render `publishDiagnostics` as squiggles + a Problems
   panel, implement completion (`textDocument/completion`) and go-to-definition (`Ctrl+Click`).
4. **Build integration:** "Build" action shells to `cmake --build`, streams output live into the
   Output panel, clicking an error line jumps to the file/location.
5. **DAP integration (lldb-dap):** breakpoint gutter clicks, `launch`, `setBreakpoints`,
   `configurationDone`, handle `stopped` events, populate call stack / scopes / variables,
   implement step over/into/out and continue.
6. **Multi-language proof:** add rust-analyzer or pyright as a second LSP backend to confirm the
   client abstraction is actually language-agnostic.
7. **Polish:** settings/theme switcher, integrated terminal, git status in file tree, recent
   projects, session restore.

## 5. Coding conventions

- One class per header/source pair, named after the class (`MainWindow.h` / `MainWindow.cpp`).
- Signals/slots for cross-component communication; avoid tight coupling between `ui/` and
  `lsp/`/`dap/` — panels should react to Qt signals emitted by client objects, not poll them.
- No business logic in `.ui` files / no Qt Designer — build layouts in code so diffs stay
  readable and the docking/theme system stays consistent.
- All user-visible strings should go through `tr()` even if localization isn't a near-term goal
  — cheap to do now, painful to retrofit later.
- Keep `LspClient`/`DapClient` protocol-agnostic of *which* language server is on the other end;
  language-specific behavior belongs in a thin config (executable path, launch args) not in the
  client class.

## 6. Definition of done for the prototype (what's in this repo right now)

- Frameless, draggable, resizable main window with custom title bar (min/max/close).
- Dark theme applied via QSS loaded from `resources/themes/dark.qss`.
- Dockable panels: file tree (left), tabbed editor (center), output panel (bottom).
- "Open Folder" populates the file tree; double-click opens a file in a new editor tab.
- Editor has line numbers and current-line highlighting (no syntax highlighting yet — that's
  milestone 2).
- Everything builds with a standard `cmake -B build -G Ninja && cmake --build build` given Qt6
  installed, no extra third-party fetch steps required.

## 7. Immediate next task for an agent picking this up

Implement **Milestone 2** (tree-sitter syntax highlighting for C++) on top of this prototype:
1. Add tree-sitter core + `tree-sitter-cpp` grammar as a CMake `FetchContent` dependency.
2. Create `TreeSitterHighlighter` that parses the buffer incrementally on `textChanged`,
   walks the resulting tree, and applies `QTextCharFormat` styling per node type via
   `QSyntaxHighlighter` (or direct `QTextCursor` formatting if `QSyntaxHighlighter`'s per-block
   model doesn't fit tree-sitter's whole-tree incremental model well — evaluate both).
3. Map a reasonable set of tree-sitter node types (`identifier`, `string_literal`, `comment`,
   `primitive_type`, keywords, etc.) to theme-driven colors defined in `dark.qss`'s custom
   properties or a small `Theme` struct.
4. Confirm re-highlighting stays fast on a large-ish file (a few thousand lines) by only
   re-parsing the edited range, not the whole file, on each keystroke.
