# Contributing to OpenIDE

Thank you for your interest in contributing to **OpenIDE**! We welcome contributions from developers of all skill levels, whether you are fixing bugs, improving documentation, adding new features, or optimizing performance.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Building from Source](#building-from-source)
- [Project Architecture](#project-architecture)
- [Coding Guidelines](#coding-guidelines)
- [Submitting Changes](#submitting-changes)
  - [Pull Request Process](#pull-request-process)
  - [Commit Message Conventions](#commit-message-conventions)

---

## Code of Conduct

This project and everyone participating in it is governed by the [OpenIDE Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior according to the instructions in the policy.

---

## Getting Started

### Prerequisites

To build OpenIDE, ensure you have the following installed on your system:

- **Compiler**: C++20 compliant compiler (MSVC 2022 v17.x+, Clang 15+, or GCC 12+)
- **Build System**: [CMake](https://cmake.org/) 3.22+ and [Ninja](https://ninja-build.org/)
- **GUI Framework**: [Qt 6.8+](https://www.qt.io/) (Widgets, Core, Gui, Network)

### Building from Source

#### Windows (MSVC + Ninja)

Run the clean build script:
```cmd
build_and_run.bat
```

Or configure and build manually:
```cmd
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.8.0/msvc2022_64"
cmake --build build
```

#### Running Executable

Launch the compiled IDE:
```cmd
run.bat
```
or directly run `build/OpenIDE.exe`.

---

## Project Architecture

The codebase is modularized under `src/`:

- **`src/app/`**: Application startup, main loop integration, and global application state.
- **`src/core/`**: Core utilities including thread-safe logging (`Logger`), configuration manager (`Config`), and platform helpers.
- **`src/editor/`**: High-performance text editor engine (`EditorWidget`), document abstractions (`DocumentManager`), snippet handling (`SnippetEngine`), and completion popup overlays (`CompletionPopup`).
- **`src/syntax/`**: Tree-sitter incremental parser binding (`TreeSitterHighlighter`) for syntax highlighting.
- **`src/language/`**: Language intelligence services (`CompletionService`, `IncludeIndex`, `LspClient`, `LspTransport`).
- **`src/lsp/`**: LSP manager (`LspManager`), diagnostics tracking (`LspDiagnosticsManager`), request routing (`LspRequestManager`), and semantic tokens (`LspSemanticTokenManager`).
- **`src/project/`**: Workspace and project root detection (`ProjectManager`, `WorkspaceManager`), and fast indexer (`ProjectIndexer`).
- **`src/cpp/`**: Compilation database support (`compile_commands.json` parser).
- **`src/unreal/`**: Unreal Engine detection (`.uproject`, UBT build targets, engine modules).
- **`src/ui/`**: JetBrains-inspired UI widgets (`MainWindow`, `LeftToolBar`, `TopToolBar`, `BreadcrumbBar`, `SearchEverywhereDialog`, `ThemeManager`).

---

## Coding Guidelines

1. **C++ Standard**: Code MUST strictly adhere to standard **C++20** constructs (`concepts`, `constexpr`, `std::filesystem`, structured bindings).
2. **Qt Best Practices**:
   - Prefer Qt signal/slot syntax: `connect(sender, &Sender::signal, receiver, &Receiver::slot)`.
   - Use `QString`, `QList`, and `QMap` appropriately for Qt UI bindings, while preferring `std` types in pure core logic.
3. **Memory Safety**: Use RAII and smart pointers (`std::unique_ptr`, `std::shared_ptr`, `QPointer`). Avoid explicit manual allocations without clear ownership lifecycle.
4. **Formatting**: Keep code cleanly formatted with 4-space indentation, consistent namespace guards (`namespace OpenIDE::Subsystem { ... }`), and descriptive variable names.
5. **No Blockers on Main Loop**: UI thread must remain responsive at all times. Offload heavy I/O, LSP RPC calls, and project indexing to background worker threads.

---

## Submitting Changes

### Pull Request Process

1. **Fork the Repository**: Create your feature branch (`git checkout -b feature/amazing-feature`).
2. **Keep Commits Atomic**: Make focused, modular commits that address a single logical change.
3. **Test Changes**: Ensure the project compiles cleanly without warnings or errors.
4. **Push & Create PR**: Push your branch to GitHub and open a Pull Request against `main`. Describe what your PR changes, why, and how to test it.

### Commit Message Conventions

We follow Conventional Commits standard:
- `feat(editor)`: Add support for inline parameter hints
- `fix(lsp)`: Resolve crash during clangd server shutdown
- `docs`: Update README setup steps for Linux
- `refactor(ui)`: Clean up theme color token management
- `style`: Fix indentation in TopToolBar.cpp

---

Thank you for helping build **OpenIDE**! 🚀
