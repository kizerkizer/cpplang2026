cmake_minimum_required(VERSION 3.20)

project(englishbreakfast LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(englishbreakfast
    src/main.cpp
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/parser/node.cpp
    src/common/programuniqueid.cpp
    src/common/sourcecodelocation.cpp
    src/common/source.cpp
    src/common/util.cpp
    src/diagnostics/diagnostics.cpp
    src/diagnostics/diagnosticmessage.cpp
    src/desugarer/desugarer.cpp
    src/binder/binder.cpp
    src/binder/symbol.cpp
    src/binder/scope.cpp
    src/checker/checker.cpp
    src/checker/type.cpp
    src/checker/typestore.cpp
    src/flowbuilder/flowgraph.cpp
    src/flowbuilder/flownode.cpp
    src/flowbuilder/flowbuilder.cpp
    src/treewalker/environment.cpp
    src/treewalker/interpreter.cpp
    src/treewalker/outputstream.cpp
    src/utf8scanner/utf8scanner.cpp
)

target_include_directories(englishbreakfast PRIVATE include)

target_compile_options(englishbreakfast PRIVATE -fsanitize=address -fno-omit-frame-pointer)
target_link_options(englishbreakfast PRIVATE -fsanitize=address)

if (MSVC)
    target_compile_options(englishbreakfast PRIVATE /W4)
else()
    target_compile_options(englishbreakfast PRIVATE -Wall -Wextra -Wpedantic)
endif()
