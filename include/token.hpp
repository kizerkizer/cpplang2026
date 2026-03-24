#pragma once
#include <string>

enum class TokenName {
    OutOfRange,
    TriviaWhitespace,
    TriviaCommentShort,
    TriviaCommentLong,
    Identifier,
    IntegerLiteral,
    StringLiteral,
    BooleanLiteral,
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
    Minus,
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
    KeywordElse,
    KeywordClass,
    KeywordWhile,
    KeywordFunction,
    KeywordReturn,
    KeywordContinue,
    KeywordBreak,
    KeywordVar,
};

#define IS_TOKENNAME_LITERAL(tokenName) (tokenName == TokenName::IntegerLiteral || tokenName == TokenName::StringLiteral || tokenName == TokenName::BooleanLiteral)
#define IS_TOKENNAME_KEYWORD(tokenName) (tokenName == TokenName::KeywordIf || tokenName == TokenName::KeywordElse || tokenName == TokenName::KeywordClass || tokenName == TokenName::KeywordWhile || tokenName == TokenName::KeywordFunction || tokenName == TokenName::KeywordReturn || tokenName == TokenName::KeywordVar)
#define IS_TOKENNAME_TRIVIA(tokenName) (tokenName == TokenName::TriviaWhitespace || tokenName == TokenName::TriviaCommentShort || tokenName == TokenName::TriviaCommentLong)
#define IS_TOKENNAME_OPERATOR(tokenName) (tokenName == TokenName::Plus || tokenName == TokenName::Minus || tokenName == TokenName::Asterisk || tokenName == TokenName::Slash || tokenName == TokenName::Equal || tokenName == TokenName::LessThan || tokenName == TokenName::GreaterThan || tokenName == TokenName::EqualEqual || tokenName == TokenName::NotEqual || tokenName == TokenName::LessThanEqual || tokenName == TokenName::GreaterThanEqual || tokenName == TokenName::And || tokenName == TokenName::Or || tokenName == TokenName::Not || tokenName == TokenName::AsteriskAsterisk)
#define IS_TOKENNAME_BINARY_OPERATOR(tokenName) (tokenName == TokenName::Plus || tokenName == TokenName::Minus || tokenName == TokenName::Asterisk || tokenName == TokenName::Slash || tokenName == TokenName::Equal || tokenName == TokenName::LessThan || tokenName == TokenName::GreaterThan || tokenName == TokenName::EqualEqual || tokenName == TokenName::NotEqual || tokenName == TokenName::LessThanEqual || tokenName == TokenName::GreaterThanEqual || tokenName == TokenName::And || tokenName == TokenName::Or || tokenName == TokenName::AsteriskAsterisk)
#define LEFT_ASSOCIATIVE true
#define RIGHT_ASSOCIATIVE false

int getPrecedence (const TokenName& tokenName);

bool getAssociativity (const TokenName& tokenName); // true for left, false for right

std::string tokenNameToString (const TokenName &name);

class Token {
public:
    Token(const std::string &sourceString,int index, int line, int column, TokenName name)
        : name(name), sourceString(sourceString), index(index), line(line), column(column) {};
    TokenName getTokenName() const;
    int getIndex() const;
    int getLine() const;
    int getColumn() const;
    std::string getSourceString() const;
    std::string toString() const;
    bool operator==(const TokenName& rhs) const;
    bool operator!=(const TokenName& rhs) const;
    friend bool operator==(const Token& lhs,const Token& rhs);
    friend bool operator!=(const Token& lhs,const Token& rhs);
private:
    TokenName name;
    std::string sourceString;
    int index;
    int line;
    int column;
};