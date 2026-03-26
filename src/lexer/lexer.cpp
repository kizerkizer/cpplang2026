#include <cctype>
#include <cstddef>
#include <map>
#include <string>
#include <print>

#include "lexer/lexer.hpp"
#include "common/sourcecodelocation.hpp"

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

Lexer::Lexer() {}

void Lexer::advance(const int &steps = 1) {
    this->index += steps;
    this->column += steps;
}

std::tuple<int, int, int> Lexer::getCurrentSourceCodeLocation() const {
    return {this->index, this->line, this->column};
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
};

std::map<std::string, TokenKind, LongestToShortestComparer> operators = {
    {"==", TokenKind::EqualEqual},
    {"!=", TokenKind::NotEqual},
    {"<=", TokenKind::LessThanEqual},
    {">=", TokenKind::GreaterThanEqual},
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
};

std::map<std::string, TokenKind, LongestToShortestComparer> punctuators = {
    {".", TokenKind::Dot},
    {",", TokenKind::Comma},
    {";", TokenKind::Semicolon},
    {"(", TokenKind::ParenthesisOpen},
    {")", TokenKind::ParenthesisClose},
    {"{", TokenKind::BraceOpen},
    {"}", TokenKind::BraceClose},
    {"[", TokenKind::BracketOpen},
    {"]", TokenKind::BracketClose},
};

std::string Lexer::makeErrorMessage(const std::string& message) const {
    return "Lexer error at line " + std::to_string(this->line) + ", column " + std::to_string(this->column) + ": " + message;
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

std::vector<Token> Lexer::lex(const std::string& sourceString, std::vector<std::string>& errorMessages_out) {
    this->sourceString = sourceString;
    std::vector<Token> tokens;
    while (!this->isPastSourceStringEnd()) {
        if (this->getCharacter() == '\r' && this->getCharacter(1) == '\n') {
            // Windows-style newline
            auto token = Token(sourceString.substr(this->index, 2), this->index, this->line, this->column, TokenKind::TriviaNewline);
            token.setLastSourceCodeLocation(SourceCodeLocation(this->index + 1, this->line, this->column + 1));
            tokens.push_back(token);
            this->advance(2);
            this->line++;
            this->column = 1;
            continue;
        }
        if (this->getCharacter() == '\n') {
            // Unix-style newline
            auto token = Token(sourceString.substr(this->index, 1), this->index, this->line, this->column, TokenKind::TriviaNewline);
            // LastSourceLocation automatically set to first in constructor
            tokens.push_back(token);
            this->advance();
            this->line++;
            this->column = 1;
            continue;
        }
        if (isWhitespace(this->getCharacter())) {
            auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
            while (isWhitespace(this->getCharacter()) && !isNewline(this->getCharacter())) {
                this->advance();
            }
            auto token = Token(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::TriviaWhitespace);
            token.setLastSourceCodeLocation(SourceCodeLocation(this->index - 1, this->line, this->column - 1));
            tokens.push_back(token);
            continue;
        }
        // Single line comment
        if (this->getCharacter() == '/' && this->getCharacter(1) == '/') {
            auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
            while (!isNewline(this->getCharacter()) && !this->isPastSourceStringEnd()) {
                this->advance();
            }
            auto token = Token(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::TriviaCommentShort);
            token.setLastSourceCodeLocation(SourceCodeLocation(this->index - 1, this->line, this->column - 1));
            tokens.push_back(token);
            continue;
        }
        // /*...*/ comment
        if (this->getCharacter() == '/' && this->getCharacter(1) == '*') {
            auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
            this->advance(2);
            while (!(this->getCharacter() == '*' && this->getCharacter(1) == '/') && !this->isPastSourceStringEnd()) {
                if (isNewline(this->getCharacter())) {
                    this->line++;
                    this->column = 1;
                }
                this->advance();
            }
            if (this->isPastSourceStringEnd()) {
                errorMessages_out.push_back(this->makeErrorMessage("Unterminated comment starting at line " + std::to_string(startLine) + ", column " + std::to_string(startColumn)));
                return tokens;
            }
            this->advance(2);
            auto token = Token(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::TriviaCommentLong);
            token.setLastSourceCodeLocation(SourceCodeLocation(this->index - 1, this->line, this->column - 1));
            tokens.push_back(token);
            continue;
        }
        // Keywords
        for (const auto& [keyword, tokenName] : keywords) {
            if (sourceString.compare(this->index, keyword.size(), keyword) == 0) {
                auto token = Token(keyword, this->index, this->line, this->column, tokenName);
                token.setLastSourceCodeLocation(SourceCodeLocation(this->index + keyword.size() - 1, this->line, this->column));
                tokens.push_back(token);
                this->advance(keyword.size());
                goto nextIteration;
            }
        }
        // Special values
        for (const auto& [value, tokenName] : specialValues) {
            if (sourceString.compare(this->index, value.size(), value) == 0) {
                auto token = Token(value, this->index, this->line, this->column, tokenName);
                token.setLastSourceCodeLocation(SourceCodeLocation(this->index + value.size() - 1, this->line, this->column));
                tokens.push_back(token);
                this->advance(value.size());
                goto nextIteration;
            }
        }
        // Identifier
        if (isIdentifierStart(this->getCharacter())) {
            auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
            this->advance();
            while (isIdentifierPart(this->getCharacter())) {
                this->advance();
            }
            Token token = Token(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::Identifier);
            token.setLastSourceCodeLocation(SourceCodeLocation(this->index - 1, this->line, this->column-1));
            tokens.push_back(token);
            continue;
        }
        // String literal (Only on one line for now)
        if (isStringLiteralQuote(this->getCharacter())) {
            auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
            this->advance();
            while (!isStringLiteralQuote(this->getCharacter())) {
                if (isNewline(this->getCharacter()) || this->isPastSourceStringEnd()) {
                    errorMessages_out.push_back(this->makeErrorMessage("Unterminated string literal"));
                    return tokens;
                } 
                if (this->getCharacter() == '\\' && isStringLiteralQuote(this->getCharacter(1))) {
                    this->advance(2);
                    continue;
                }
                this->advance();
            }
            this->advance();
            auto token = Token(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::LiteralString);
            token.setLastSourceCodeLocation(SourceCodeLocation(this->index - 1, this->line, this->column - 1));
            tokens.push_back(token);
            continue;
        }
        // Integer literal
        if (isIntegerLiteral(this->getCharacter())) {
            auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
            this->advance();
            while (isIntegerLiteral(this->getCharacter())) {
                this->advance();
            }
            auto token = Token(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::LiteralInteger);
            token.setLastSourceCodeLocation(SourceCodeLocation(this->index - 1, this->line, this->column - 1));
            tokens.push_back(token);
            continue;
        }
        // Operators
        for (const auto& [op, tokenName] : operators) {
            if (sourceString.compare(this->index, op.size(), op) == 0) {
                auto token = Token(op, this->index, this->line, this->column, tokenName);
                token.setLastSourceCodeLocation(SourceCodeLocation(this->index + op.size() - 1, this->line, this->column));
                tokens.push_back(token);
                this->advance(op.size());
                goto nextIteration;
            }
        }
        // Punctuators
        for (const auto& [punctuator, tokenName] : punctuators) {
            if (this->getCharacter() == punctuator[0]) {
                auto token = Token(punctuator, this->index, this->line, this->column, tokenName);
                token.setLastSourceCodeLocation(SourceCodeLocation(this->index + punctuator.size() - 1, this->line, this->column));
                tokens.push_back(token);
                this->advance(1);
                goto nextIteration;
            }
        }
        errorMessages_out.push_back(this->makeErrorMessage(std::string("Unexpected character '") + this->getCharacter() + "'"));
        return tokens;
        nextIteration:;
    }
    return tokens;
}