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
public:
    Lexer(Utf8Scanner& scanner, Source* source, Diagnostics& diagnostics) : scanner(scanner), source(source), diagnostics(diagnostics) {}
    std::unique_ptr<Token> getNextToken();
    std::unique_ptr<Token> getNextNonTrivialToken();
    bool isDone();
private:
    Utf8Scanner& scanner;
    Source* source;
    Diagnostics& diagnostics;
    // All of this now in utf8scanner:
        //std::string sourceString;
        /*size_t index = 0;
        size_t line = 1;
        size_t column = 1;
        void advance(const int &steps);*/
        //SourceCodeLocation getCurrentSourceCodeLocation() const;
        //char getCharacter (const int &offset) const;
        //bool isPastSourceStringEnd () const;
    void addDiagnostic(DiagnosticMessageKind kind, int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    void addError(int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    void addWarning(int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    void addInfo(int code, const std::string& message, std::optional<SourceCodeLocation> location = std::nullopt);
    std::unique_ptr<Token> makeToken(TokenKind tokenKind, std::string_view sourceString, std::optional<SourceCodeLocation> startSourceCodeLocation = std::nullopt, std::optional<SourceCodeLocation> endSourceCodeLocation = std::nullopt);
    std::unique_ptr<Token> makeTokenAndAdvance(TokenKind tokenKind, std::string_view sourceString, std::optional<SourceCodeLocation> startSourceCodeLocation = std::nullopt, std::optional<SourceCodeLocation> endSourceCodeLocation = std::nullopt);
};