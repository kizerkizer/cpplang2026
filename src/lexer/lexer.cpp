#include <cctype>
#include <cstddef>
#include <map>
#include <string>
#include <print>

#include "lexer/lexer.hpp"

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

std::tuple<size_t, size_t, size_t> Lexer::getCurrentSourceCodeLocation() const {
    return {this->index, this->line, this->column};
}

std::map<std::string, TokenKind> keywords = {
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

std::map<std::string, TokenKind> specialValues = {
    {"true", TokenKind::LiteralBoolean},
    {"false", TokenKind::LiteralBoolean},
    {"empty", TokenKind::LiteralEmpty},
};

std::map<std::string, TokenKind> operators = {
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

std::map<std::string, TokenKind> punctuators = {
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
            tokens.emplace_back(sourceString.substr(this->index, 2), this->index, this->line, this->column, TokenKind::TriviaNewline);
            this->advance(2);
            this->line++;
            this->column = 1;
            continue;
        }
        if (this->getCharacter() == '\n') {
            // Unix-style newline
            tokens.emplace_back(sourceString.substr(this->index, 1), this->index, this->line, this->column, TokenKind::TriviaNewline);
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
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::TriviaWhitespace);
            continue;
        }
        // Single line comment
        if (this->getCharacter() == '/' && this->getCharacter(1) == '/') {
            auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
            while (!isNewline(this->getCharacter()) && !this->isPastSourceStringEnd()) {
                this->advance();
            }
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::TriviaCommentShort);
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
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::TriviaCommentLong);
            continue;
        }
        // Keywords
        for (const auto& [keyword, tokenName] : keywords) {
            if (sourceString.compare(this->index, keyword.size(), keyword) == 0) {
                tokens.emplace_back(keyword, this->index, this->line, this->column, tokenName);
                this->advance(keyword.size());
                goto nextIteration;
            }
        }
        // Special values
        for (const auto& [value, tokenName] : specialValues) {
            if (sourceString.compare(this->index, value.size(), value) == 0) {
                tokens.emplace_back(value, this->index, this->line, this->column, tokenName);
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
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::Identifier);
            continue;
        }
        // String literal
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
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::LiteralString);
            continue;
        }
        if (isIntegerLiteral(this->getCharacter())) {
            auto [startIndex, startLine, startColumn] = this->getCurrentSourceCodeLocation();
            this->advance();
            while (isIntegerLiteral(this->getCharacter())) {
                this->advance();
            }
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenKind::LiteralInteger);
            continue;
        }
        // Operators
        for (const auto& [op, tokenName] : operators) {
            if (sourceString.compare(this->index, op.size(), op) == 0) {
                tokens.emplace_back(op, this->index, this->line, this->column, tokenName);
                this->advance(op.size());
                goto nextIteration;
            }
        }
        // Punctuators
        for (const auto& [punctuator, tokenName] : punctuators) {
            if (this->getCharacter() == punctuator[0]) {
                tokens.emplace_back(punctuator, this->index, this->line, this->column, tokenName);
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