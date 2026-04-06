#pragma once

#include <string>
#include <memory>
#include <optional>

#include "common/source.hpp"
#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnosticmessage.hpp"
#include "diagnostics/diagnostics.hpp"
#include "lexer/token.hpp"
#include "utf8scanner/utf8scanner.hpp"

class Lexer {
private:
    Utf8Scanner& scanner;
    Source* source;
    Diagnostics& diagnostics;
    void addDiagnostic(DiagnosticMessageKind kind, int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    void addError(int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    void addWarning(int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    void addInfo(int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    std::unique_ptr<Token> makeToken(TokenKind tokenKind, std::string_view sourceString, std::optional<SourceCodeLocation> startSourceCodeLocation = std::nullopt, std::optional<SourceCodeLocation> endSourceCodeLocation = std::nullopt);
    std::unique_ptr<Token> makeTokenAndAdvance(TokenKind tokenKind, std::string_view sourceString, std::optional<SourceCodeLocation> startSourceCodeLocation = std::nullopt, std::optional<SourceCodeLocation> endSourceCodeLocation = std::nullopt);
public:
    Lexer(Utf8Scanner& scanner, Source* source, Diagnostics& diagnostics) : scanner(scanner), source(source), diagnostics(diagnostics) {}
    std::unique_ptr<Token> getNextToken();
    std::unique_ptr<Token> getNextNonTrivialToken();
    bool isDone();
};