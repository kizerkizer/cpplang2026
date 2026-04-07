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


bool isIdentifierStart (char32_t c) {
    return std::isalpha(c) || c == '_';
}

bool isIdentifierPart (char32_t c) {
    return std::isalnum(c) || c == '_' || c == '-';
}

bool isIntegerLiteral (char32_t c) {
    return std::isdigit(c);
}

bool isWhitespace (char32_t c) {
    return std::isspace(c);
}

bool isNewline (char32_t c) {
    return c == '\n' || c == '\r';
}

bool isStringLiteralQuote (char32_t c) {
    return c == '\'';
}

// Lexer
bool Lexer::isDone() {
    return m_scanner.isDone();
}

void Lexer::reset() {
    m_scanner.reset();
}

std::unique_ptr<Token> Lexer::makeToken(TokenKind tokenKind, std::string_view sourceString, std::optional<SourceCodeLocation> startSourceCodeLocation, std::optional<SourceCodeLocation> endSourceCodeLocation) {
    if (tokenKind == TokenKind::OutOfRange) {
        return std::make_unique<Token>(m_source, sourceString, SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1)), tokenKind);
    }
    if (!startSourceCodeLocation.has_value()) {
        startSourceCodeLocation = m_scanner.getCurrentSourceCodeLocation();
    }
    auto [byteIndex, codepointIndex, line, column] = startSourceCodeLocation.value();
    auto endLocation = endSourceCodeLocation.has_value() ? endSourceCodeLocation.value() : SourceCodeLocation(byteIndex + sourceString.size() - 1, codepointIndex + sourceString.size() - 1, line, column + sourceString.size() - 1);
    SourceCodeLocationSpan sourceCodeLocationSpan(startSourceCodeLocation.value(), endLocation);
    auto token = std::make_unique<Token>(m_source, sourceString, sourceCodeLocationSpan, tokenKind);
    return token;
}

std::unique_ptr<Token> Lexer::makeTokenAndAdvance(TokenKind tokenKind, std::string_view sourceString, std::optional<SourceCodeLocation> startSourceCodeLocation, std::optional<SourceCodeLocation> endSourceCodeLocation) {
    auto token = this->makeToken(tokenKind, sourceString, startSourceCodeLocation, endSourceCodeLocation);
    m_scanner.advance(sourceString.size());
    return token;
}

void Lexer::addDiagnostic(DiagnosticMessageKind kind, int code, const std::string& message, std::optional<SourceCodeLocation> location) {
    auto loc = location.has_value() ? location.value() : m_scanner.getCurrentSourceCodeLocation();
    auto locationSpan = SourceCodeLocationSpan(loc, loc);
    auto diagnosticMessage = DiagnosticMessage(code, kind, DiagnosticMessageStage::Lexer, locationSpan, m_source, message);
    m_diagnostics.addDiagnosticMessage(diagnosticMessage);
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

std::unique_ptr<Token> Lexer::getNextNonTrivialToken() {
    auto token = this->getNextToken();
    while (IS_TOKENKIND_TRIVIA(token->getTokenKind())) {
        token = this->getNextToken();
    }
    return token;
}

std::unique_ptr<Token> Lexer::getNextToken() {
    if (this->isDone()) {
        return this->makeToken(TokenKind::OutOfRange, "");
    }
    std::unique_ptr<Token> nextToken = nullptr;
    if (this->m_scanner.peekCodepoint() == '\r' && this->m_scanner.peekCodepoint(1) == '\n') {
        // Windows-style newline
        auto token = this->makeToken(TokenKind::TriviaNewline, this->m_scanner.substr(this->m_scanner.getByteIndex(), 2));
        nextToken = std::move(token);
        m_scanner.advance(2);
        return nextToken;
    }
    if (m_scanner.peekCodepoint() == '\n') {
        // Unix-style newline
        auto token = this->makeToken(TokenKind::TriviaNewline, m_scanner.substr(m_scanner.getByteIndex(), 1));
        // LastSourceLocation automatically set to first in constructor
        nextToken = std::move(token);
        m_scanner.advance();
        return nextToken;
    }
    if (isWhitespace(m_scanner.peekCodepoint())) {
        auto [startByteIndex, startCodepointIndex, startLine, startColumn] = m_scanner.getCurrentSourceCodeLocation();
        while (isWhitespace(m_scanner.peekCodepoint()) && !isNewline(m_scanner.peekCodepoint())) {
            m_scanner.advance();
        }
        auto startLocation = SourceCodeLocation(startByteIndex, startCodepointIndex, startLine, startColumn);
        auto token = this->makeToken(TokenKind::TriviaWhitespace, m_scanner.substr(startByteIndex, m_scanner.getByteIndex() - startByteIndex), startLocation);
        nextToken = std::move(token);
        return nextToken;
    }
    // Single line comment
    if (m_scanner.peekCodepoint() == '/' && m_scanner.peekCodepoint(1) == '/') {
        auto [startByteIndex, startCodepointIndex, startLine, startColumn] = m_scanner.getCurrentSourceCodeLocation();
        while (!isNewline(m_scanner.peekCodepoint()) && !m_scanner.isDone()) {
            m_scanner.advance();
        }
        auto startLocation = SourceCodeLocation(startByteIndex, startCodepointIndex, startLine, startColumn);
        auto token = this->makeToken(TokenKind::TriviaCommentShort, m_scanner.substr(startByteIndex, m_scanner.getByteIndex() - startByteIndex), startLocation);
        nextToken = std::move(token);
        return nextToken;
    }
    // /*...*/ comment
    if (m_scanner.peekCodepoint() == '/' && m_scanner.peekCodepoint(1) == '*') {
        auto [startByteIndex, startCodepointIndex, startLine, startColumn] = m_scanner.getCurrentSourceCodeLocation();
        m_scanner.advance(2);
        while (!(m_scanner.peekCodepoint() == '*' && m_scanner.peekCodepoint(1) == '/') && !m_scanner.isDone()) {
            if (m_scanner.peekCodepoint() == '\r' && m_scanner.peekCodepoint(1) == '\n') {
                m_scanner.advance(2);
                continue;
            }
            if (m_scanner.peekCodepoint() == '\n') {
                m_scanner.advance();
                continue;
            }
            m_scanner.advance();
        }
        if (m_scanner.isDone()) {
            this->addError(2, "Unterminated comment", SourceCodeLocation(startByteIndex - 1, startCodepointIndex - 1, startLine, startColumn - 1));
            return nextToken;
        }
        m_scanner.advance(2);
        auto startLocation = SourceCodeLocation(startByteIndex, startCodepointIndex, startLine, startColumn);
        auto endLocation = SourceCodeLocation(m_scanner.getByteIndex() - 1, m_scanner.getCodepointIndex() - 1, m_scanner.getLine(), m_scanner.getColumn() - 1);
        auto token = this->makeToken(TokenKind::TriviaCommentLong, m_scanner.substr(startByteIndex, m_scanner.getByteIndex() - startByteIndex), startLocation, endLocation);
        nextToken = std::move(token);
        return nextToken;
    }
    // Identifier
    if (isIdentifierStart(m_scanner.peekCodepoint())) {
        auto [startByteIndex, startCodepointIndex, startLine, startColumn] = m_scanner.getCurrentSourceCodeLocation();
        m_scanner.advance();
        while (isIdentifierPart(m_scanner.peekCodepoint())) {
            m_scanner.advance();
        }
        auto sourceString = m_scanner.substr(startByteIndex, m_scanner.getByteIndex() - startByteIndex);

        // check keywords
        auto keywordIt = keywords.find(std::string(sourceString));
        if (keywordIt != keywords.end()) {
            auto token = this->makeToken(keywordIt->second, sourceString, SourceCodeLocation(startByteIndex, startCodepointIndex, startLine, startColumn));
            nextToken = std::move(token);
            return nextToken;
        }

        // check special values
        auto specialIt = specialValues.find(std::string(sourceString));
        if (specialIt != specialValues.end()) {
            auto token = this->makeToken(specialIt->second, sourceString, SourceCodeLocation(startByteIndex, startCodepointIndex, startLine, startColumn));
            nextToken = std::move(token);
            return nextToken;
        }

        // otherwise, it's an identifier
        auto startLocation = SourceCodeLocation(startByteIndex, startCodepointIndex, startLine, startColumn);
        auto token = this->makeToken(TokenKind::Identifier, sourceString, startLocation);
        nextToken = std::move(token);
        return nextToken;
    }

    // String literal (Only on one line for now)
    if (isStringLiteralQuote(m_scanner.peekCodepoint())) {
        auto [startByteIndex, startCodepointIndex, startLine, startColumn] = m_scanner.getCurrentSourceCodeLocation();
        m_scanner.advance();
        while (!isStringLiteralQuote(m_scanner.peekCodepoint())) {
            if (isNewline(m_scanner.peekCodepoint()) || m_scanner.isDone()) {
                this->addError(3, "Unterminated string literal", SourceCodeLocation(startByteIndex - 1, startCodepointIndex - 1, startLine, startColumn - 1));
                return nextToken;
            } 
            if (m_scanner.peekCodepoint() == '\\' && isStringLiteralQuote(m_scanner.peekCodepoint(1))) {
                m_scanner.advance(2);
                continue;
            }
            m_scanner.advance();
        }
        m_scanner.advance();
        auto startLocation = SourceCodeLocation(startByteIndex, startCodepointIndex, startLine, startColumn);
        auto token = this->makeToken(TokenKind::LiteralString, m_scanner.substr(startByteIndex, m_scanner.getByteIndex() - startByteIndex), startLocation);
        nextToken = std::move(token);
        return nextToken;
    }

    // Integer literal
    if (isIntegerLiteral(m_scanner.peekCodepoint())) {
        auto [startByteIndex, startCodepointIndex, startLine, startColumn] = m_scanner.getCurrentSourceCodeLocation();
        m_scanner.advance();
        while (isIntegerLiteral(m_scanner.peekCodepoint())) {
            m_scanner.advance();
        }
        auto startLocation = SourceCodeLocation(startByteIndex, startCodepointIndex, startLine, startColumn);
        auto token = this->makeToken(TokenKind::LiteralInteger, m_scanner.substr(startByteIndex, m_scanner.getByteIndex() - startByteIndex), startLocation);
        nextToken = std::move(token);
        return nextToken;
    }

    // Operators
    for (const auto& [op, tokenName] : operators) {
        if (m_scanner.substr(m_scanner.getByteIndex(), op.size()).compare(0, op.size(), op) == 0) {
            auto token = this->makeTokenAndAdvance(tokenName, op);
            nextToken = std::move(token);
            return nextToken;
        }
    }

    // Punctuators
    for (const auto& [punctuator, tokenName] : punctuators) {
        if (m_scanner.peekCodepoint() == (char32_t)punctuator[0]) {
            auto token = this->makeTokenAndAdvance(tokenName, punctuator);
            nextToken = std::move(token);
            return nextToken;
        }
    }
    this->addError(4, std::string("Unexpected character '") + std::string(1, m_scanner.peekCodepoint()) + "'", m_scanner.getCurrentSourceCodeLocation());
    return nextToken;
}