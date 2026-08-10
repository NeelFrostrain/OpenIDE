# MyIDE — prototype (Milestone 1: shell)

Native Qt6 IDE shell: frameless window, custom title bar, dark flat theme,
dockable file tree + output panels, tabbed plain-text editor with line
numbers and current-line highlight.

See `PROMPT.md` for the full architecture/spec and next-milestone plan
(syntax highlighting via tree-sitter, LSP via clangd, DAP via lldb-dap).

## Build

Requires Qt 6 (Widgets), CMake ≥ 3.21, Ninja, a C++20 compiler.

```bash
# Debian/Ubuntu
sudo apt install qt6-base-dev cmake ninja-build g++

cmake -B build -G Ninja
cmake --build build
./build/myide
```

This has been built and smoke-tested (headless, `QT_QPA_PLATFORM=offscreen`)
in this environment — it compiles clean and starts its event loop without
crashing. It has not been visually verified on a real display; do that first
when you pick this up.

## What's here

- `src/ui/TitleBar` — frameless-window title bar with min/max/close + drag-to-move
- `src/ui/MainWindow` — dock layout (file tree left, output bottom, tabbed editors center), menu bar, theme loader
- `src/ui/FileTreePanel` — `QFileSystemModel`-backed project tree, emits `fileActivated`
- `src/ui/EditorWidget` — `QPlainTextEdit` + line-number gutter + current-line highlight
- `src/ui/OutputPanel` — read-only log widget for build/console output (not wired to a real build yet)
- `resources/themes/dark.qss` — the whole visual theme, edit this to restyle

## What's NOT here yet (see PROMPT.md milestones 2+)

- Syntax highlighting (tree-sitter)
- LSP integration (clangd — completion, diagnostics, go-to-def)
- Build integration (cmake/ninja invocation + error parsing)
- DAP integration (lldb-dap — breakpoints, stepping, variables)
- QScintilla migration (currently `QPlainTextEdit`, fine for the shell milestone)
