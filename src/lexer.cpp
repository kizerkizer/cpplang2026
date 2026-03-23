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

bool isNewline (char c) {
    return c == '\n' || c == '\r';
}

bool isIntegerLiteral (char c) {
    return std::isdigit(c);
}

bool isWhitespace (char c) {
    return std::isspace(c);
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
    {"else", TokenName::KeywordElse},
    {"class", TokenName::KeywordClass},
    {"while", TokenName::KeywordWhile},
    {"function", TokenName::KeywordFunction},
    {"return", TokenName::KeywordReturn},
    {"continue", TokenName::KeywordContinue},
    {"break", TokenName::KeywordBreak},
    {"var", TokenName::KeywordVar},
};

std::map<std::string, TokenName> operators = {
    {"+", TokenName::Plus},
    {"-", TokenName::Minus},
    {"*", TokenName::Asterisk},
    {"/", TokenName::Slash},
    {"=", TokenName::Equal},
    {"<", TokenName::LessThan},
    {">", TokenName::GreaterThan},
    {"==", TokenName::EqualEqual},
    {"!=", TokenName::NotEqual},
    {"<=", TokenName::LessThanEqual},
    {">=", TokenName::GreaterThanEqual},
    {"&&", TokenName::And},
    {"||", TokenName::Or},
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
        char c = this->getCharacter();
        if (isNewline(this->getCharacter())) {
            this->advance();
            this->line++;
            this->column = 1; // TODO change to resetColumn()
            continue;
        }
        if (isWhitespace(this->getCharacter())) {
            auto [startIndex, startLine, startColumn] = this->getCounters();
            while (isWhitespace(this->getCharacter()) && !isNewline(this->getCharacter())) {
                this->advance();
            }
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenName::Whitespace);
            continue;
        }
        for (const auto& [keyword, tokenName] : keywords) {
            if (sourceString.compare(this->index, keyword.size(), keyword) == 0) {
                tokens.emplace_back(keyword, this->index, this->line, this->column, tokenName);
                this->advance(keyword.size());
                goto nextIteration;
            }
        }
        if (isIdentifierStart(this->getCharacter())) {
            auto [startIndex, startLine, startColumn] = this->getCounters();
            this->advance();
            while (isIdentifierPart(this->getCharacter())) {
                this->advance();
            }
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenName::Identifier);
            continue;
        }
        if (isDoubleQuote(c)) {
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
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenName::StringLiteral);
            continue;
        }
        if (isIntegerLiteral(c)) {
            auto [startIndex, startLine, startColumn] = this->getCounters();
            this->advance();
            while (isIntegerLiteral(this->getCharacter())) {
                this->advance();
            }
            tokens.emplace_back(sourceString.substr(startIndex, this->index - startIndex), startIndex, startLine, startColumn, TokenName::IntegerLiteral);
            continue;
        }
        for (const auto& [op, tokenName] : operators) {
            if (sourceString.compare(this->index, op.size(), op) == 0) {
                tokens.emplace_back(op, this->index, this->line, this->column, tokenName);
                this->advance(op.size());
                goto nextIteration;
            }
        }
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