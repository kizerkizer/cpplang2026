#pragma once
#include <string>

#include "common/sourcecodelocation.hpp"
#include "common/source.hpp"

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
    Colon,
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
    RightArrow,
    LeftArrow,
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
    TypePrimitiveBoolean,
    TypePrimitiveNumber,
    TypePrimitiveInteger,
    TypePrimitiveFloat,
    TypePrimitiveString,
    TypePrimitiveEmpty,
};

#define IS_TOKENKIND_LITERAL(tokenKind) (tokenKind == TokenKind::IntegerLiteral || tokenKind == TokenKind::StringLiteral || tokenKind == TokenKind::BooleanLiteral)
#define IS_TOKENKIND_KEYWORD(tokenKind) (tokenKind == TokenKind::KeywordIf || tokenKind == TokenKind::KeywordThen || tokenKind == TokenKind::KeywordElse || tokenKind == TokenKind::KeywordClass || tokenKind == TokenKind::KeywordWhile || tokenKind == TokenKind::KeywordLoop || tokenKind == TokenKind::KeywordFunction || tokenKind == TokenKind::KeywordReturn || tokenKind == TokenKind::KeywordVar || tokenKind == TokenKind::KeywordType)
#define IS_TOKENKIND_TRIVIA(tokenKind) (tokenKind == TokenKind::TriviaWhitespace || tokenKind == TokenKind::TriviaCommentShort || tokenKind == TokenKind::TriviaCommentLong || tokenKind == TokenKind::TriviaNewline)
#define IS_TOKENKIND_OPERATOR(tokenKind) (tokenKind == TokenKind::Dot || tokenKind == TokenKind::Plus || tokenKind == TokenKind::Dash || tokenKind == TokenKind::Asterisk || tokenKind == TokenKind::Slash || tokenKind == TokenKind::Equal || tokenKind == TokenKind::LessThan || tokenKind == TokenKind::GreaterThan || tokenKind == TokenKind::EqualEqual || tokenKind == TokenKind::NotEqual || tokenKind == TokenKind::LessThanEqual || tokenKind == TokenKind::GreaterThanEqual || tokenKind == TokenKind::And || tokenKind == TokenKind::Or || tokenKind == TokenKind::Not || tokenKind == TokenKind::AsteriskAsterisk)
#define IS_TOKENKIND_BINARY_OPERATOR(tokenKind) (tokenKind == TokenKind::Dot || tokenKind == TokenKind::Plus || tokenKind == TokenKind::Dash || tokenKind == TokenKind::Asterisk || tokenKind == TokenKind::Slash || tokenKind == TokenKind::Equal || tokenKind == TokenKind::LessThan || tokenKind == TokenKind::GreaterThan || tokenKind == TokenKind::EqualEqual || tokenKind == TokenKind::NotEqual || tokenKind == TokenKind::LessThanEqual || tokenKind == TokenKind::GreaterThanEqual || tokenKind == TokenKind::And || tokenKind == TokenKind::Or || tokenKind == TokenKind::AsteriskAsterisk)
#define LEFT_ASSOCIATIVE true
#define RIGHT_ASSOCIATIVE false

int getPrecedence (const TokenKind& tokenName);

bool getAssociativity (const TokenKind& tokenName); // true for left, false for right. Use LEFT_ASSOCIATIVE and RIGHT_ASSOCIATIVE macros for readability

std::string tokenNameToString (const TokenKind &name);

class Token {
public:
    Token(Source* source, const std::string &sourceString, SourceCodeLocationSpan sourceCodeLocationSpan, TokenKind name, bool compilerCreated = false)
        : source(source), name(name), sourceString(sourceString), sourceCodeLocationSpan(sourceCodeLocationSpan), compilerCreated(compilerCreated) {};
    TokenKind getTokenKind() const;
    Source* getSource() const;
    /*int getFirstIndex() const;
    int getFirstLine() const;
    int getFirstColumn() const;
    int getLastIndex() const;
    int getLastLine() const;
    int getLastColumn() const;*/
    std::string getSourceString() const;
    std::string toString() const;
    bool operator==(const TokenKind& rhs) const;
    bool operator!=(const TokenKind& rhs) const;
    friend bool operator==(const Token& lhs,const Token& rhs);
    friend bool operator!=(const Token& lhs,const Token& rhs);
    bool isCompilerCreated() const;
    SourceCodeLocationSpan getSourceCodeLocationSpan() const;
    /*void setLastSourceCodeLocation(SourceCodeLocation location);
    void setFirstSourceCodeLocation(SourceCodeLocation location);
    void setLastSourceCodeLocation(SourceCodeLocation location);
    SourceCodeLocation getFirstSourceCodeLocation() const;
    SourceCodeLocation getLastSourceCodeLocation() const;*/
private:
    Source* source;
    TokenKind name;
    std::string sourceString;
    SourceCodeLocationSpan sourceCodeLocationSpan;
    /*int firstIndex;
    int firstLine;
    int firstColumn;
    int lastIndex;
    int lastLine;
    int lastColumn;*/
    bool compilerCreated;
};