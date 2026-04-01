cmake_minimum_required(VERSION 3.20)

project(cpplang2026 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(cpplang2026
    src/main.cpp
    src/lexer/token.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/parser/node.cpp
    src/common/programuniqueid.cpp
    src/common/sourcecodelocation.cpp
    src/common/source.cpp
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
)

target_include_directories(cpplang2026 PRIVATE include)

target_compile_options(cpplang2026 PRIVATE -fsanitize=address -fno-omit-frame-pointer)
target_link_options(cpplang2026 PRIVATE -fsanitize=address)

if (MSVC)
    target_compile_options(cpplang2026 PRIVATE /W4)
else()
    target_compile_options(cpplang2026 PRIVATE -Wall -Wextra -Wpedantic)
endif()
