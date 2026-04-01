#include <cctype>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <print>

#include "lexer/lexer.hpp"
#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnosticmessage.hpp"
#include "lexer/token.hpp"

bool isIdentifierStart (char c) {
    return std::isalpha(c) || c == '_';
}

bool isIdentifierPart (char c) {
    return std::isalnum(c) || c == '_' || c == '-';
}

bool isIntegerLiteral (char c) {
    return std::isdigit(c);
}

bool isWhitespace (char c) {
    return std::isspace(c);
}

bool isNewline (char c) {
    return c == '\n' || c == '\r';
}

bool isStringLiteralQuote (char c) {
    return c == '\'';
}

void Lexer::advance(const int &steps = 1) {
    this->index += steps;
    this->column += steps;
}

std::tuple<int, int, int> Lexer::getCurrentSourceCodeLocation() const {
    return {this->index, this->line, this->column};
}

std::unique_ptr<Token> Lexer::makeToken(TokenKind tokenKind, std::string sourceString, std::optional<SourceCodeLocation> startSourceCodeLocation, std::optional<SourceCodeLocation> endSourceCodeLocation) {
    if (tokenKind == TokenKind::OutOfRange) {
        return std::make_unique<Token>(this->source, sourceString, SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1)), tokenKind);
    }
    if (!startSourceCodeLocation.has_value()) {
        startSourceCodeLocation = this->getCurrentSourceCodeLocation();
    }
    auto [index, line, column] = startSourceCodeLocation.value();
    auto endLocation = endSourceCodeLocation.has_value() ? endSourceCodeLocation.value() : SourceCodeLocation(index + sourceString.size() - 1, line, column + sourceString.size() - 1);
    SourceCodeLocationSpan sourceCodeLocationSpan(startSourceCodeLocation.value(), endLocation);
    auto token = std::make_unique<Token>(this->source, sourceString, sourceCodeLocationSpan, tokenKind);
    return token;
}

std::unique_ptr<Token> Lexer::makeTokenAndAdvance(TokenKind tokenKind, std::string sourceString, std::optional<SourceCodeLocation> startSourceCodeLocation, std::optional<SourceCodeLocation> endSourceCodeLocation) {
    auto token = this->makeToken(tokenKind, sourceString, startSourceCodeLocation, endSourceCodeLocation);
    this->advance(sourceString.size());
    return token;
}

struct LongestToShortestComparer {
    bool operator()(const std::string& left, const std::string& right) const {
        if (left.length() != right.length()) {
            return left.length() > right.length();
        }
        return left < right;
    }
};

std::map<std::string, TokenKind, LongestToShortestComparer> keywords = {
    {"if", TokenKind::KeywordIf},
    {"then", TokenKind::KeywordThen},
    {"else", TokenKind::KeywordElse},
    {"class", TokenKind::KeywordClass},
    {"while", TokenKind::KeywordWhile},
    {"loop", TokenKind::KeywordLoop},
    {"function", TokenKind::KeywordFunction},
    {"return", TokenKind::KeywordReturn},
    {"continue", TokenKind::KeywordContinue},
    {"break", TokenKind::KeywordBreak},
    {"var", TokenKind::KeywordVar},
    {"bind", TokenKind::KeywordBind},
    {"type", TokenKind::KeywordType},
    {"extensional", TokenKind::KeywordExtensional},
    {"intensional", TokenKind::KeywordIntensional},
};

std::map<std::string, TokenKind, LongestToShortestComparer> specialValues = {
    {"true", TokenKind::LiteralBoolean},
    {"false", TokenKind::LiteralBoolean},
    {"empty", TokenKind::LiteralEmpty},
    {"Boolean", TokenKind::TypePrimitiveBoolean},
    {"Integer", TokenKind::TypePrimitiveInteger},
    {"Float", TokenKind::TypePrimitiveFloat},
    {"String", TokenKind::TypePrimitiveString},
    {"Empty", TokenKind::TypePrimitiveEmpty},
    {"Number", TokenKind::TypePrimitiveNumber},
};

std::map<std::string, TokenKind, LongestToShortestComparer> operators = {
    {"==", TokenKind::EqualEqual},
    {"!=", TokenKind::NotEqual},
    {"<=", TokenKind::LessThanEqual},
    {">=", TokenKind::GreaterThanEqual},
    {"->", TokenKind::RightArrow},
    {"<-", TokenKind::LeftArrow},
    {"&&", TokenKind::And},
    {"||", TokenKind::Or},
    {"**", TokenKind::AsteriskAsterisk},
    {"+", TokenKind::Plus},
    {"-", TokenKind::Dash},
    {"*", TokenKind::Asterisk},
    {"/", TokenKind::Slash},
    {"=", TokenKind::Equal},
    {"<", TokenKind::LessThan},
    {">", TokenKind::GreaterThan},
    {"!", TokenKind::Not},
    {".", TokenKind::Dot},
};

std::map<std::string, TokenKind, LongestToShortestComparer> punctuators = {
    {",", TokenKind::Comma},
    {":", TokenKind::Colon},
    {";", TokenKind::Semicolon},
    {"(", TokenKind::ParenthesisOpen},
    {")", TokenKind::ParenthesisClose},
    {"{", TokenKind::BraceOpen},
    {"}", TokenKind::BraceClose},
    {"[", TokenKind::BracketOpen},
    {"]", TokenKind::BracketClose},
};

void Lexer::addDiagnostic(DiagnosticMessageKind kind, int code, const std::string& message, std::optional<SourceCodeLocation> location) {
    auto loc = location.has_value() ? location.value() : SourceCodeLocation(this->index, this->line, this->column);
    auto locationSpan = SourceCodeLocationSpan(loc, loc);
    auto diagnosticMessage = DiagnosticMessage(code, kind, DiagnosticMessageStage::Lexer, locationSpan, this->source, message);
    this->diagnostics.addDiagnosticMessage(diagnosticMessage);
}

void Lexer::addError(int code, const std::string& message, std::optional<SourceCodeLocation> location) {
    this->addDiagnostic(DiagnosticMessageKind::Error, code, message, location);
}

void Lexer::addWarning(int code, const std::string& message, std::optional<SourceCodeLocation> location) {
    this->addDiagnostic(DiagnosticMessageKind::Warning, code, message, location);
}

void Lexer::addInfo(int code, const std::string& message, std::optional<SourceCodeLocation> location) {
    this->addDiagnostic(DiagnosticMessageKind::Info, code, message, location);
}

char Lexer::getCharacter(const int &offset = 0) const {
    if (this->index + offset >= this->sourceString.size()) {
        return '\0';
    }
    return this->sourceString[this->index + offset];
}

bool Lexer::isPastSourceStringEnd() const {
    return this->index >= this->sourceString.size();
}

bool Lexer::isDone() {
    return this->isPastSourceStringEnd();
}

std::unique_ptr<Token> Lexer::getNextNonTrivialToken() {
    auto token = this->getNextToken();
    while (IS_TOKENKIND_TRIVIA(token->getTokenKind())) {
        token = this->getNextToken();
    }
    return token;
}

std::unique_ptr<Token> Lexer::getNextToken() {
    if (this->isPastSourceStringEnd()) {
        return this->makeToken(TokenKind::OutOfRange, "");
    }
    std::unique_ptr<Token> nextToken = nullptr;
    if (this->getCharacter() == '\r' && this->getCharacter(1) == '\n') {
        // Windows-style newline
        auto token = this->makeToken(TokenKind::TriviaNewline, sourceString.substr(this->index, 2));
        nextToken = std::move(token);
        this->advance(2);
        this->line++;
        this->column = 1;
        return nextToken;
    }
    if (this->getCharacter() == '\n') {
        // Unix-style newline
        auto token = this->makeToken(TokenKind::TriviaNewline, sourceString.substr(this->index, 1));
        // LastSourceLocation automatically set to first in constructor
        nextToken = std::move(token);
        this->advance();
        this->line++;
        this->column = 1;
        return nextToken;
    }
    if (isWhitespace(this->getCharacter())) {
        auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
        while (isWhitespace(this->getCharacter()) && !isNewline(this->getCharacter())) {
            this->advance();
        }
        auto startLocation = SourceCodeLocation(startIndex, startLine, startColumn);
        auto token = this->makeToken(TokenKind::TriviaWhitespace, sourceString.substr(startIndex, this->index - startIndex), startLocation);
        nextToken = std::move(token);
        return nextToken;
    }
    // Single line comment
    if (this->getCharacter() == '/' && this->getCharacter(1) == '/') {
        auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
        while (!isNewline(this->getCharacter()) && !this->isPastSourceStringEnd()) {
            this->advance();
        }
        auto startLocation = SourceCodeLocation(startIndex, startLine, startColumn);
        auto token = this->makeToken(TokenKind::TriviaCommentShort, sourceString.substr(startIndex, this->index - startIndex), startLocation);
        nextToken = std::move(token);
        return nextToken;
    }
    // /*...*/ comment
    if (this->getCharacter() == '/' && this->getCharacter(1) == '*') {
        auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
        this->advance(2);
        while (!(this->getCharacter() == '*' && this->getCharacter(1) == '/') && !this->isPastSourceStringEnd()) {
            if (this->getCharacter() == '\r' && this->getCharacter(1) == '\n') {
                this->advance(2);
                this->line++;
                this->column = 1;
                continue;
            }
            if (this->getCharacter() == '\n') {
                this->advance();
                this->line++;
                this->column = 1;
                continue;
            }
            this->advance();
        }
        if (this->isPastSourceStringEnd()) {
            this->addError(2, "Unterminated comment", SourceCodeLocation(startIndex - 1, startLine, startColumn - 1));
            return nextToken;
        }
        this->advance(2);
        auto startLocation = SourceCodeLocation(startIndex, startLine, startColumn);
        auto endLocation = SourceCodeLocation(this->index - 1, this->line, this->column - 1);
        auto token = this->makeToken(TokenKind::TriviaCommentLong, sourceString.substr(startIndex, this->index - startIndex), startLocation, endLocation);
        nextToken = std::move(token);
        return nextToken;
    }
    // Keywords
    for (const auto& [keyword, tokenName] : keywords) {
        if (sourceString.compare(this->index, keyword.size(), keyword) == 0) {
            auto token = this->makeTokenAndAdvance(tokenName, keyword);
            nextToken = std::move(token);
            return nextToken;
        }
    }
    // Special values
    for (const auto& [value, tokenName] : specialValues) {
        if (sourceString.compare(this->index, value.size(), value) == 0) {
            auto token = this->makeTokenAndAdvance(tokenName, value);
            nextToken = std::move(token);
            return nextToken;
        }
    }
    // Identifier
    if (isIdentifierStart(this->getCharacter())) {
        auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
        this->advance();
        while (isIdentifierPart(this->getCharacter())) {
            this->advance();
        }
        auto startLocation = SourceCodeLocation(startIndex, startLine, startColumn);
        auto token = this->makeToken(TokenKind::Identifier, sourceString.substr(startIndex, this->index - startIndex), startLocation);
        nextToken = std::move(token);
        return nextToken;
    }
    // String literal (Only on one line for now)
    if (isStringLiteralQuote(this->getCharacter())) {
        auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
        this->advance();
        while (!isStringLiteralQuote(this->getCharacter())) {
            if (isNewline(this->getCharacter()) || this->isPastSourceStringEnd()) {
                this->addError(3, "Unterminated string literal", SourceCodeLocation(startIndex - 1, startLine, startColumn - 1));
                return nextToken;
            } 
            if (this->getCharacter() == '\\' && isStringLiteralQuote(this->getCharacter(1))) {
                this->advance(2);
                continue;
            }
            this->advance();
        }
        this->advance();
        auto startLocation = SourceCodeLocation(startIndex, startLine, startColumn);
        auto token = this->makeToken(TokenKind::LiteralString, sourceString.substr(startIndex, this->index - startIndex), startLocation);
        nextToken = std::move(token);
        return nextToken;
    }
    // Integer literal
    if (isIntegerLiteral(this->getCharacter())) {
        auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
        this->advance();
        while (isIntegerLiteral(this->getCharacter())) {
            this->advance();
        }
        auto startLocation = SourceCodeLocation(startIndex, startLine, startColumn);
        auto token = this->makeToken(TokenKind::LiteralInteger, sourceString.substr(startIndex, this->index - startIndex), startLocation);
        nextToken = std::move(token);
        return nextToken;
    }
    // Operators
    for (const auto& [op, tokenName] : operators) {
        if (sourceString.compare(this->index, op.size(), op) == 0) {
            auto token = this->makeTokenAndAdvance(tokenName, op);
            nextToken = std::move(token);
            return nextToken;
        }
    }
    // Punctuators
    for (const auto& [punctuator, tokenName] : punctuators) {
        if (this->getCharacter() == punctuator[0]) {
            auto token = this->makeTokenAndAdvance(tokenName, punctuator);
            nextToken = std::move(token);
            return nextToken;
        }
    }
    this->addError(4, std::string("Unexpected character '") + this->getCharacter() + "'", this->getCurrentSourceCodeLocation());
    return nextToken;
}