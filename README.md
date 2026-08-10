# MyIDE — Lightweight Native C++ & Unreal Engine IDE

MyIDE is a high-performance, lightweight native desktop IDE built with **C++20** and **Qt 6 Widgets**, designed specifically for **C++**, **Unreal Engine**, and modern multi-project C++ development.

## Features

- **Fast & Responsive**: Built natively using Qt 6 Widgets and C++20.
- **Tree-sitter Syntax Highlighting**: Fast, parser-based incremental syntax highlighting for C++.
- **LSP Code Intelligence**: Powered by `clangd` (autocompletion, diagnostics, hover, go-to definition, find references, document symbols).
- **Unreal Engine Integration**: `.uproject` awareness, source modules, UnrealBuildTool support.
- **CMake & Ninja Build Engine**: Asynchronous compile streaming and clickable diagnostic navigation.
- **DAP Debugging**: Debug Adapter Protocol integration for native debugging (`lldb-dap`).

## Building

### Requirements
- C++20 compliant compiler (MSVC 2022 / Clang / GCC)
- CMake 3.22+
- Ninja build system
- Qt 6.8+ (Widgets, Core, Gui, Network)

### Build Commands (Windows)
```cmd
cmake -B build -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.8.0/msvc2022_64"
cmake --build build
```
