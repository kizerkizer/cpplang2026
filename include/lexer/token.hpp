#pragma once
#include "common/sourcecodelocation.hpp"
#include <string>

enum class TokenKind {
    OutOfRange,
    TriviaWhitespace,
    TriviaNewline,
    TriviaCommentShort,
    TriviaCommentLong,
    Identifier,
    LiteralInteger,
    LiteralString,
    LiteralBoolean,
    LiteralEmpty,
    Dot,
    Comma,
    Semicolon,
    ParenthesisOpen,
    ParenthesisClose,
    BraceOpen,
    BraceClose,
    BracketOpen,
    BracketClose,
    Plus,
    Dash,
    Asterisk,
    AsteriskAsterisk,
    Slash,
    Equal,
    LessThan,
    GreaterThan,
    EqualEqual,
    NotEqual,
    LessThanEqual,
    GreaterThanEqual,
    And,
    Or,
    Not,
    KeywordIf,
    KeywordThen,
    KeywordElse,
    KeywordClass,
    KeywordWhile,
    KeywordLoop,
    KeywordFunction,
    KeywordReturn,
    KeywordContinue,
    KeywordBreak,
    KeywordVar,
    KeywordBind,
    KeywordType,
    KeywordExtensional,
    KeywordIntensional,
};

#define IS_TOKENNAME_LITERAL(tokenName) (tokenName == TokenKind::IntegerLiteral || tokenName == TokenKind::StringLiteral || tokenName == TokenKind::BooleanLiteral)
#define IS_TOKENNAME_KEYWORD(tokenName) (tokenName == TokenKind::KeywordIf || tokenName == TokenKind::KeywordThen || tokenName == TokenKind::KeywordElse || tokenName == TokenKind::KeywordClass || tokenName == TokenKind::KeywordWhile || tokenName == TokenKind::KeywordLoop || tokenName == TokenKind::KeywordFunction || tokenName == TokenKind::KeywordReturn || tokenName == TokenKind::KeywordVar || tokenName == TokenKind::KeywordType)
#define IS_TOKENNAME_TRIVIA(tokenName) (tokenName == TokenKind::TriviaWhitespace || tokenName == TokenKind::TriviaCommentShort || tokenName == TokenKind::TriviaCommentLong || tokenName == TokenKind::TriviaNewline)
#define IS_TOKENNAME_OPERATOR(tokenName) (tokenName == TokenKind::Plus || tokenName == TokenKind::Dash || tokenName == TokenKind::Asterisk || tokenName == TokenKind::Slash || tokenName == TokenKind::Equal || tokenName == TokenKind::LessThan || tokenName == TokenKind::GreaterThan || tokenName == TokenKind::EqualEqual || tokenName == TokenKind::NotEqual || tokenName == TokenKind::LessThanEqual || tokenName == TokenKind::GreaterThanEqual || tokenName == TokenKind::And || tokenName == TokenKind::Or || tokenName == TokenKind::Not || tokenName == TokenKind::AsteriskAsterisk)
#define IS_TOKENNAME_BINARY_OPERATOR(tokenName) (tokenName == TokenKind::Plus || tokenName == TokenKind::Dash || tokenName == TokenKind::Asterisk || tokenName == TokenKind::Slash || tokenName == TokenKind::Equal || tokenName == TokenKind::LessThan || tokenName == TokenKind::GreaterThan || tokenName == TokenKind::EqualEqual || tokenName == TokenKind::NotEqual || tokenName == TokenKind::LessThanEqual || tokenName == TokenKind::GreaterThanEqual || tokenName == TokenKind::And || tokenName == TokenKind::Or || tokenName == TokenKind::AsteriskAsterisk)
#define LEFT_ASSOCIATIVE true
#define RIGHT_ASSOCIATIVE false

int getPrecedence (const TokenKind& tokenName);

bool getAssociativity (const TokenKind& tokenName); // true for left, false for right. Use LEFT_ASSOCIATIVE and RIGHT_ASSOCIATIVE macros for readability

std::string tokenNameToString (const TokenKind &name);

class Token {
public:
    Token(const std::string &sourceString, size_t index, size_t line, size_t column, TokenKind name, bool compilerCreated = false)
        : name(name), sourceString(sourceString), firstIndex(index), firstLine(line), firstColumn(column), lastIndex(index), lastLine(line), lastColumn(column), compilerCreated(compilerCreated) {};
    TokenKind getTokenName() const;
    int getFirstIndex() const;
    int getFirstLine() const;
    int getFirstColumn() const;
    int getLastIndex() const;
    int getLastLine() const;
    int getLastColumn() const;
    std::string getSourceString() const;
    std::string toString() const;
    bool operator==(const TokenKind& rhs) const;
    bool operator!=(const TokenKind& rhs) const;
    friend bool operator==(const Token& lhs,const Token& rhs);
    friend bool operator!=(const Token& lhs,const Token& rhs);
    bool isCompilerCreated() const;
    void setFirstSourceCodeLocation(SourceCodeLocation location);
    void setLastSourceCodeLocation(SourceCodeLocation location);
    SourceCodeLocation getFirstSourceCodeLocation() const;
    SourceCodeLocation getLastSourceCodeLocation() const;
private:
    TokenKind name;
    std::string sourceString;
    int firstIndex;
    int firstLine;
    int firstColumn;
    int lastIndex;
    int lastLine;
    int lastColumn;
    bool compilerCreated;
};