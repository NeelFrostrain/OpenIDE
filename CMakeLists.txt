cmake_minimum_required(VERSION 3.22)
project(OpenIDE VERSION 0.1.0 LANGUAGES C CXX)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable AUTOMOC, AUTORCC, and AUTOUIC for Qt6
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network)

# Fetch nlohmann_json for LSP & DAP messaging
include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# Fetch Tree-sitter C runtime & C++ grammar parser
FetchContent_Declare(
    tree-sitter
    URL https://github.com/tree-sitter/tree-sitter/archive/refs/tags/v0.22.6.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Declare(
    tree-sitter-cpp
    URL https://github.com/tree-sitter/tree-sitter-cpp/archive/refs/tags/v0.22.3.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Tree-sitter build definitions
FetchContent_GetProperties(tree-sitter)
if(NOT tree-sitter_POPULATED)
    FetchContent_Populate(tree-sitter)
    add_library(tree_sitter_runtime STATIC
        "${tree-sitter_SOURCE_DIR}/lib/src/lib.c"
    )
    target_include_directories(tree_sitter_runtime PUBLIC
        "${tree-sitter_SOURCE_DIR}/lib/include"
        "${tree-sitter_SOURCE_DIR}/lib/src"
    )
endif()

FetchContent_GetProperties(tree-sitter-cpp)
if(NOT tree-sitter-cpp_POPULATED)
    FetchContent_Populate(tree-sitter-cpp)
    add_library(tree_sitter_cpp STATIC
        "${tree-sitter-cpp_SOURCE_DIR}/src/parser.c"
        "${tree-sitter-cpp_SOURCE_DIR}/src/scanner.c"
    )
    target_include_directories(tree_sitter_cpp PUBLIC
        "${tree-sitter-cpp_SOURCE_DIR}/src"
    )
    target_link_libraries(tree_sitter_cpp PRIVATE tree_sitter_runtime)
endif()


# Compiler options
if(MSVC)
    add_compile_options(/W4 /MP /utf-8 /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Source files
set(SOURCES
    # Main & App
    src/main.cpp
    src/app/Application.h
    src/app/Application.cpp

    # Core
    src/core/Logger.h
    src/core/Logger.cpp
    src/core/Config.h
    src/core/Config.cpp

    # Editor
    src/editor/ITextEditor.h
    src/editor/Document.h
    src/editor/Document.cpp
    src/editor/DocumentManager.h
    src/editor/DocumentManager.cpp
    src/editor/EditorWidget.h
    src/editor/EditorWidget.cpp
    src/editor/CompletionPopup.h
    src/editor/CompletionPopup.cpp
    src/editor/CompletionController.h
    src/editor/CompletionController.cpp
    src/editor/snippets/SnippetDefinition.h
    src/editor/snippets/SnippetParser.h
    src/editor/snippets/SnippetParser.cpp
    src/editor/snippets/SnippetRegistry.h
    src/editor/snippets/SnippetRegistry.cpp
    src/editor/snippets/SnippetSession.h
    src/editor/snippets/SnippetSession.cpp
    src/editor/snippets/SnippetEngine.h
    src/editor/snippets/SnippetEngine.cpp

    # Syntax
    src/syntax/TreeSitterHighlighter.h
    src/syntax/TreeSitterHighlighter.cpp

    # Language (LSP & Autocomplete)
    src/language/CompletionContext.h
    src/language/CompletionContext.cpp
    src/language/CompletionRanking.h
    src/language/CompletionRanking.cpp
    src/language/CompletionService.h
    src/language/CompletionService.cpp
    src/language/IncludeIndex.h
    src/language/IncludeIndex.cpp
    src/language/LspTransport.h
    src/language/LspTransport.cpp
    src/language/LspClient.h
    src/language/LspClient.cpp

    # Project
    src/project/ProjectManager.h
    src/project/ProjectManager.cpp
    src/project/WorkspaceManager.h
    src/project/WorkspaceManager.cpp
    src/project/ProjectIndexer.h
    src/project/ProjectIndexer.cpp

    # C++ & Unreal Language Intelligence Subsystem
    src/lsp/LspTypes.h
    src/lsp/LspRequestManager.h
    src/lsp/LspRequestManager.cpp
    src/lsp/LspDiagnosticsManager.h
    src/lsp/LspDiagnosticsManager.cpp
    src/lsp/LspSemanticTokenManager.h
    src/lsp/LspSemanticTokenManager.cpp
    src/lsp/LspManager.h
    src/lsp/LspManager.cpp

    src/cpp/CompilationDatabase.h
    src/cpp/CompilationDatabase.cpp
    src/unreal/UnrealProjectDetector.h
    src/unreal/UnrealProjectDetector.cpp

    # UI
    src/ui/ThemeManager.h
    src/ui/ThemeManager.cpp
    src/ui/LeftToolBar.h
    src/ui/LeftToolBar.cpp
    src/ui/TopToolBar.h
    src/ui/TopToolBar.cpp
    src/ui/ToolWindowHeader.h
    src/ui/ToolWindowHeader.cpp
    src/ui/ProjectItemDelegate.h
    src/ui/ProjectItemDelegate.cpp
    src/ui/BreadcrumbBar.h
    src/ui/BreadcrumbBar.cpp
    src/ui/SearchEverywhereDialog.h
    src/ui/SearchEverywhereDialog.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
)

add_executable(OpenIDE ${SOURCES})

target_link_libraries(OpenIDE PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
    nlohmann_json::nlohmann_json
    tree_sitter_runtime
    tree_sitter_cpp
)

target_include_directories(OpenIDE PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
