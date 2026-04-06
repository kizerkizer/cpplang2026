#pragma once

#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnostics.hpp"
#include <string>
#include <stddef.h>

class Utf8Scanner {
public:
    Utf8Scanner(Source* source, Diagnostics& diagnostics);
    char32_t peekCodepoint(size_t offset = 0);
    char32_t nextCodepoint();
    void advance(size_t count = 1);
    std::string_view substr(size_t byteIndex, size_t byteLength);
    bool isDone() const;
    size_t getByteIndex() const;
    size_t getCodepointIndex() const;
    size_t getLine() const;
    size_t getColumn() const;
    SourceCodeLocation getCurrentSourceCodeLocation() const;
private:
    Source* source;
    std::string input; // TODO should be a reference eventually
    size_t byteIndex = 0;
    size_t codepointIndex = 0;
    size_t line = 1;
    size_t column = 1;
    Diagnostics& diagnostics;
    void addError(std::string message, SourceCodeLocationSpan sourceCodeLocationSpan);
};