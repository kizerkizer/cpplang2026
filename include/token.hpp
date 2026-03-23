#pragma once
#include <string>

enum class TokenName {
    OutOfRange,
    Whitespace,
    Comment,
    Identifier,
    IntegerLiteral,
    StringLiteral,
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

#define IS_TOKENNAME_LITERAL(tokenName) (tokenName == TokenName::IntegerLiteral || tokenName == TokenName::StringLiteral)
#define IS_TOKENNAME_KEYWORD(tokenName) (tokenName == TokenName::KeywordIf || tokenName == TokenName::KeywordElse || tokenName == TokenName::KeywordClass || tokenName == TokenName::KeywordWhile || tokenName == TokenName::KeywordFunction || tokenName == TokenName::KeywordReturn || tokenName == TokenName::KeywordVar)
#define IS_TOKENNAME_TRIVIA(tokenName) (tokenName == TokenName::Whitespace || tokenName == TokenName::Comment)

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