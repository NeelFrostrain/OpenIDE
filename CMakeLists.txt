cmake_minimum_required(VERSION 3.21)
project(MyIDE LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()

include(FetchContent)

FetchContent_Declare(
    tree_sitter
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
    GIT_TAG v0.22.6
)

FetchContent_Declare(
    tree_sitter_cpp
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-cpp.git
    GIT_TAG v0.22.0
)

FetchContent_GetProperties(tree_sitter)
if(NOT tree_sitter_POPULATED)
    FetchContent_Populate(tree_sitter)
endif()

FetchContent_GetProperties(tree_sitter_cpp)
if(NOT tree_sitter_cpp_POPULATED)
    FetchContent_Populate(tree_sitter_cpp)
endif()

add_library(tree-sitter STATIC ${tree_sitter_SOURCE_DIR}/lib/src/lib.c)
set_target_properties(tree-sitter PROPERTIES LINKER_LANGUAGE C)
set_source_files_properties(${tree_sitter_SOURCE_DIR}/lib/src/lib.c PROPERTIES LANGUAGE C)
target_include_directories(tree-sitter PUBLIC ${tree_sitter_SOURCE_DIR}/lib/include ${tree_sitter_SOURCE_DIR}/lib/src)

add_library(tree-sitter-cpp STATIC ${tree_sitter_cpp_SOURCE_DIR}/src/parser.c ${tree_sitter_cpp_SOURCE_DIR}/src/scanner.c)
set_target_properties(tree-sitter-cpp PROPERTIES LINKER_LANGUAGE C)
set_source_files_properties(${tree_sitter_cpp_SOURCE_DIR}/src/parser.c ${tree_sitter_cpp_SOURCE_DIR}/src/scanner.c PROPERTIES LANGUAGE C)
target_include_directories(tree-sitter-cpp PUBLIC ${tree_sitter_cpp_SOURCE_DIR}/src ${tree_sitter_SOURCE_DIR}/lib/include)

add_executable(myide
    src/main.cpp
    src/core/SymbolIndex.cpp
    src/core/SymbolIndex.h
    src/core/SymbolIndexer.cpp
    src/core/SymbolIndexer.h
    src/vcs/GitManager.cpp
    src/vcs/GitManager.h
    src/refactor/RefactorEngine.cpp
    src/refactor/RefactorEngine.h
    src/test/TestRunner.cpp
    src/test/TestRunner.h
    src/editor/SyntaxTheme.cpp
    src/editor/SyntaxTheme.h
    src/editor/TreeSitterHighlighter.cpp
    src/editor/TreeSitterHighlighter.h
    src/lsp/LspTypes.h
    src/lsp/LspClient.cpp
    src/lsp/LspClient.h
    src/ui/TitleBar.cpp
    src/ui/TitleBar.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/FileTreePanel.cpp
    src/ui/FileTreePanel.h
    src/ui/EditorWidget.cpp
    src/ui/EditorWidget.h
    src/ui/OutputPanel.cpp
    src/ui/OutputPanel.h
    src/ui/ProblemsPanel.cpp
    src/ui/ProblemsPanel.h
    src/ui/CompletionPopup.cpp
    src/ui/CompletionPopup.h
    src/ui/SearchEverywhereDialog.cpp
    src/ui/SearchEverywhereDialog.h
    src/ui/NavigationBar.cpp
    src/ui/NavigationBar.h
    src/ui/StructurePanel.cpp
    src/ui/StructurePanel.h
    src/ui/GitPanel.cpp
    src/ui/GitPanel.h
    src/dap/DapClient.cpp
    src/dap/DapClient.h
    src/dap/DapTypes.h
    src/refactor/RenameDialog.cpp
    src/refactor/RenameDialog.h
    src/ui/CodeMinimap.cpp
    src/ui/CodeMinimap.h
    src/ui/TerminalPanel.cpp
    src/ui/TerminalPanel.h
    src/ui/DebugToolbar.cpp
    src/ui/DebugToolbar.h
    src/ui/VariablesPanel.cpp
    src/ui/VariablesPanel.h
    src/ui/CallStackPanel.cpp
    src/ui/CallStackPanel.h
    src/ui/CMakeBar.cpp
    src/ui/CMakeBar.h
    src/ui/TestRunnerPanel.cpp
    src/ui/TestRunnerPanel.h
    src/ui/ActivityBar.cpp
    src/ui/ActivityBar.h
    src/ui/StatusBar.cpp
    src/ui/StatusBar.h
    resources/resources.qrc
)

target_include_directories(myide PRIVATE src ${tree_sitter_SOURCE_DIR}/lib/include ${tree_sitter_cpp_SOURCE_DIR}/src)

target_link_libraries(myide PRIVATE Qt6::Widgets tree-sitter tree-sitter-cpp)

if(WIN32)
    set_target_properties(myide PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
