#include <cctype>
#include <cstddef>
#include <map>
#include <string>
#include <print>

#include "lexer.hpp"

bool isIdentifierStart (char c) {
    return std::isalpha(c) || c == '_';
}

bool isIdentifierPart (char c) {
    return std::isalnum(c) || c == '_';
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

bool isDoubleQuote (char c) {
    return c == '"';
}

Lexer::Lexer() {}

void Lexer::advance(const int &steps = 1) {
    this->index += steps;
    this->column += steps;
}

std::tuple<size_t, size_t, size_t> Lexer::getCounters() const {
    return {this->index, this->line, this->column};
}

std::map<std::string, TokenName> keywords = {
    {"if", TokenName::KeywordIf},
    {"then", TokenName::KeywordThen},
    {"else", TokenName::KeywordElse},
    {"class", TokenName::KeywordClass},
    {"while", TokenName::KeywordWhile},
    {"function", TokenName::KeywordFunction},
    {"return", TokenName::KeywordReturn},
    {"continue", TokenName::KeywordContinue},
    {"break", TokenName::KeywordBreak},
    {"var", TokenName::KeywordVar},
    {"bind", TokenName::KeywordBind},
    {"type", TokenName::KeywordType},
};

std::map<std::string, TokenName> specialValues = {
    {"true", TokenName::LiteralBoolean},
    {"false", TokenName::LiteralBoolean},
    {"empty", TokenName::LiteralEmpty},
};

std::map<std::string, TokenName> operators = {
    {"==", TokenName::EqualEqual},
    {"!=", TokenName::NotEqual},
    {"<=", TokenName::LessThanEqual},
    {">=", TokenName::GreaterThanEqual},
    {"&&", TokenName::And},
    {"||", TokenName::Or},
    {"**", TokenName::AsteriskAsterisk},
    {"+", TokenName::Plus},
    {"-", TokenName::Dash},
    {"*", TokenName::Asterisk},
    {"/", TokenName::Slash},
    {"=", TokenName::Equal},
    {"<", TokenName::LessThan},
    {">", TokenName::GreaterThan},
    {"!", TokenName::Not},
};

std::map<std::string, TokenName> punctuators = {
    {".", TokenName::Dot},
    {",", TokenName::Comma},
    {";", TokenName::Semicolon},
    {"(", TokenName::ParenthesisOpen},
    {")", TokenName::ParenthesisClose},
    {"{", TokenName::BraceOpen},
    {"}", TokenName::BraceClose},
    {"[", TokenName::BracketOpen},
    {"]", TokenName::BracketClose},
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
            this->advance(2);
            this->line++;
            this->column = 1;
            continue;
        }
        if (this->getCharacter() == '\n') {
            // Unix-style newline
            this->advance();
            this->line++;
            this->column = 1;
            continue;
        }
        if (isWhitespace(this->getCharacter())) {
            auto [startIndex, startLine, startColumn] = this->getCounters();
            while (isWhitespace(this->getCharacter()) && !isNewline(this->getCharacter())) {
                this->advance();
            }
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenName::TriviaWhitespace);
            continue;
        }
        // Single line comment
        if (this->getCharacter() == '/' && this->getCharacter(1) == '/') {
            auto [startIndex, startLine, startColumn] = this->getCounters();
            while (!isNewline(this->getCharacter()) && !this->isPastSourceStringEnd()) {
                this->advance();
            }
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenName::TriviaCommentShort);
            continue;
        }
        // /*...*/ comment
        if (this->getCharacter() == '/' && this->getCharacter(1) == '*') {
            auto [startIndex, startLine, startColumn] = this->getCounters();
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
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenName::TriviaCommentLong);
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
            auto [startIndex, startLine, startColumn] = this->getCounters();
            this->advance();
            while (isIdentifierPart(this->getCharacter())) {
                this->advance();
            }
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenName::Identifier);
            continue;
        }
        // String literal
        if (isDoubleQuote(this->getCharacter())) {
            auto [startIndex, startLine, startColumn] = this->getCounters();
            this->advance();
            while (!isDoubleQuote(this->getCharacter())) {
                if (isNewline(this->getCharacter()) || this->isPastSourceStringEnd()) {
                    errorMessages_out.push_back(this->makeErrorMessage("Unterminated string literal"));
                    return tokens;
                } 
                if (this->getCharacter() == '\\' && this->getCharacter(1) == '"') {
                    this->advance(2);
                    continue;
                }
                this->advance();
            }
            this->advance();
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenName::LiteralString);
            continue;
        }
        if (isIntegerLiteral(this->getCharacter())) {
            auto [startIndex, startLine, startColumn] = this->getCounters();
            this->advance();
            while (isIntegerLiteral(this->getCharacter())) {
                this->advance();
            }
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenName::LiteralInteger);
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