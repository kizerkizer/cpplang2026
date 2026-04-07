#pragma once

#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnostics.hpp"
#include <string>
#include <stddef.h>

class Utf8Scanner {
private:
    Source* m_source;
    std::string m_input; // TODO should be a reference eventually
    size_t m_byteIndex = 0;
    size_t m_codepointIndex = 0;
    size_t m_line = 1;
    size_t m_column = 1;
    Diagnostics& m_diagnostics;
    void addError(std::string message, SourceCodeLocationSpan sourceCodeLocationSpan);
public:
    Utf8Scanner(Source* source, Diagnostics& diagnostics);
    char32_t peekCodepoint(size_t offset = 0);
    // char32_t nextCodepoint();
    void advance(size_t count = 1);
    std::string_view substr(size_t byteIndex, size_t byteLength);
    bool isDone() const;
    void reset();
    size_t getByteIndex() const;
    size_t getCodepointIndex() const;
    size_t getLine() const;
    size_t getColumn() const;
    SourceCodeLocation getCurrentSourceCodeLocation() const;
};