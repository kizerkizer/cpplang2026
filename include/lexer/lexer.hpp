#pragma once

#include <string>
#include <memory>
#include <optional>

#include "common/source.hpp"
#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnosticmessage.hpp"
#include "diagnostics/diagnostics.hpp"
#include "token.hpp"

class Lexer {
public:
    Lexer(Source* source, Diagnostics& diagnostics) : source(source), diagnostics(diagnostics), sourceString(source->getSourceString()) {}
    std::unique_ptr<Token> getNextToken();
    std::unique_ptr<Token> getNextNonTrivialToken();
    bool isDone();
private:
    Source* source;
    Diagnostics& diagnostics;
    std::string sourceString;
    size_t index = 0;
    size_t line = 1;
    size_t column = 1;
    void advance(const int &steps);
    SourceCodeLocation getCurrentSourceCodeLocation() const;
    char getCharacter (const int &offset) const;
    bool isPastSourceStringEnd () const;
    void addDiagnostic(DiagnosticMessageKind kind, int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    void addError(int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    void addWarning(int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    void addInfo(int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    std::unique_ptr<Token> makeToken(TokenKind tokenKind, std::string sourceString, std::optional<SourceCodeLocation> startSourceCodeLocation = std::nullopt, std::optional<SourceCodeLocation> endSourceCodeLocation = std::nullopt);
    std::unique_ptr<Token> makeTokenAndAdvance(TokenKind tokenKind, std::string sourceString, std::optional<SourceCodeLocation> startSourceCodeLocation = std::nullopt, std::optional<SourceCodeLocation> endSourceCodeLocation = std::nullopt);
};