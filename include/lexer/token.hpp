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

#define IS_TOKENKIND_LITERAL(tokenKind) (tokenKind == TokenKind::LiteralInteger || tokenKind == TokenKind::LiteralString || tokenKind == TokenKind::LiteralBoolean)
#define IS_TOKENKIND_KEYWORD(tokenKind) (tokenKind == TokenKind::KeywordIf || tokenKind == TokenKind::KeywordThen || tokenKind == TokenKind::KeywordElse || tokenKind == TokenKind::KeywordClass || tokenKind == TokenKind::KeywordWhile || tokenKind == TokenKind::KeywordLoop || tokenKind == TokenKind::KeywordFunction || tokenKind == TokenKind::KeywordReturn || tokenKind == TokenKind::KeywordVar || tokenKind == TokenKind::KeywordType)
#define IS_TOKENKIND_TRIVIA(tokenKind) (tokenKind == TokenKind::TriviaWhitespace || tokenKind == TokenKind::TriviaCommentShort || tokenKind == TokenKind::TriviaCommentLong || tokenKind == TokenKind::TriviaNewline)
#define IS_TOKENKIND_OPERATOR(tokenKind) (tokenKind == TokenKind::Dot || tokenKind == TokenKind::Plus || tokenKind == TokenKind::Dash || tokenKind == TokenKind::Asterisk || tokenKind == TokenKind::Slash || tokenKind == TokenKind::Equal || tokenKind == TokenKind::LessThan || tokenKind == TokenKind::GreaterThan || tokenKind == TokenKind::EqualEqual || tokenKind == TokenKind::NotEqual || tokenKind == TokenKind::LessThanEqual || tokenKind == TokenKind::GreaterThanEqual || tokenKind == TokenKind::And || tokenKind == TokenKind::Or || tokenKind == TokenKind::Not || tokenKind == TokenKind::AsteriskAsterisk)
#define IS_TOKENKIND_BINARY_OPERATOR(tokenKind) (tokenKind == TokenKind::Dot || tokenKind == TokenKind::Plus || tokenKind == TokenKind::Dash || tokenKind == TokenKind::Asterisk || tokenKind == TokenKind::Slash || tokenKind == TokenKind::Equal || tokenKind == TokenKind::LessThan || tokenKind == TokenKind::GreaterThan || tokenKind == TokenKind::EqualEqual || tokenKind == TokenKind::NotEqual || tokenKind == TokenKind::LessThanEqual || tokenKind == TokenKind::GreaterThanEqual || tokenKind == TokenKind::And || tokenKind == TokenKind::Or || tokenKind == TokenKind::AsteriskAsterisk)

constexpr int getPrecedence(const TokenKind &tokenName) {
    switch (tokenName) {
        /*case TokenName::Not:
            return 9;*/
            // Unary operators are handled separately in the parser, so we don't assign them a precedence here
        case TokenKind::Dot:
            return 9;
        case TokenKind::AsteriskAsterisk:
            return 8;
        case TokenKind::Asterisk:
        case TokenKind::Slash:
            return 7;
        case TokenKind::Plus:
        case TokenKind::Dash:
            return 6;
        case TokenKind::LessThan:
        case TokenKind::GreaterThan:
        case TokenKind::LessThanEqual:
        case TokenKind::GreaterThanEqual:
            return 5;
        case TokenKind::EqualEqual:
        case TokenKind::NotEqual:
            return 4;
        case TokenKind::And:
            return 3;
        case TokenKind::Or:
            return 2;
        case TokenKind::Equal:
            return 1;
        default: return -1;
    }
}

#define LEFT_ASSOCIATIVE true
#define RIGHT_ASSOCIATIVE false

constexpr bool getAssociativity(const TokenKind &tokenName) {
    switch (tokenName) {
        case TokenKind::Not:
        case TokenKind::Equal:
        case TokenKind::AsteriskAsterisk:
            return RIGHT_ASSOCIATIVE; // right associative
        case TokenKind::Plus:
        case TokenKind::Dash:
        case TokenKind::Asterisk:
        case TokenKind::Slash:
        case TokenKind::EqualEqual:
        case TokenKind::NotEqual:
        case TokenKind::LessThan:
        case TokenKind::GreaterThan:
        case TokenKind::LessThanEqual:
        case TokenKind::GreaterThanEqual:
            return LEFT_ASSOCIATIVE; // left associative
        default: return LEFT_ASSOCIATIVE; // default to left associative
    }
}

constexpr const char* tokenKindToString(const TokenKind &tokenKind) {
    switch (tokenKind) {
        using enum TokenKind;
        case OutOfRange: return "OutOfRange";
        case TriviaWhitespace: return "Whitespace";
        case TriviaNewline: return "Newline";
        case TriviaCommentShort: return "CommentShort";
        case TriviaCommentLong: return "CommentLong";
        case Identifier: return "Identifier";
        case LiteralBoolean: return "BooleanLiteral";
        case LiteralInteger: return "IntegerLiteral";
        case LiteralString: return "StringLiteral";
        case LiteralEmpty: return "EmptyLiteral";
        case Dot: return "Dot";
        case Comma: return "Comma";
        case Colon: return "Colon";
        case Semicolon: return "Semicolon";
        case ParenthesisOpen: return "ParenthesisOpen";
        case ParenthesisClose: return "ParenthesisClose";
        case BraceOpen: return "BraceOpen";
        case BraceClose: return "BraceClose";
        case BracketOpen: return "BracketOpen";
        case BracketClose: return "BracketClose";
        case Plus: return "Plus";
        case Dash: return "Dash";
        case Asterisk: return "Asterisk";
        case AsteriskAsterisk: return "AsteriskAsterisk";
        case Slash: return "Slash";
        case Equal: return "Equal";
        case LessThan: return "LessThan";
        case GreaterThan: return "GreaterThan";
        case LeftArrow: return "LeftArrow";
        case RightArrow: return "RightArrow";
        case EqualEqual: return "EqualEqual";
        case NotEqual: return "NotEqual";
        case LessThanEqual: return "LessThanEqual";
        case GreaterThanEqual: return "GreaterThanEqual";
        case And: return "And";
        case Or: return "Or";
        case Not: return "Not";
        case KeywordIf: return "KeywordIf";
        case KeywordThen: return "KeywordThen";
        case KeywordElse: return "KeywordElse";
        case KeywordClass: return "KeywordClass";
        case KeywordWhile: return "KeywordWhile";
        case KeywordLoop: return "KeywordLoop";
        case KeywordFunction: return "KeywordFunction";
        case KeywordReturn: return "KeywordReturn";
        case KeywordContinue: return "KeywordContinue";
        case KeywordBreak: return "KeywordBreak";
        case KeywordVar: return "KeywordVar";
        case KeywordBind: return "KeywordBind";
        case KeywordType: return "KeywordType";
        case KeywordExtensional: return "KeywordExtensional";
        case KeywordIntensional: return "KeywordIntensional";
        case TypePrimitiveBoolean: return "TypePrimitiveBoolean";
        case TypePrimitiveEmpty: return "TypePrimitveEmpty";
        case TypePrimitiveFloat: return "TypePrimitveFloat";
        case TypePrimitiveInteger: return "TypePrimitiveInteger";
        case TypePrimitiveNumber: return "TypePrimitiveNumber";
        case TypePrimitiveString: return "TypePrimitiveString";
    }
}

class Token {
private:
    Source* m_source;
    TokenKind m_name;
    std::string_view m_sourceString;
    SourceCodeLocationSpan m_sourceCodeLocationSpan;
    bool m_compilerCreated;
public:
    Token(Source* source, const std::string_view sourceString, SourceCodeLocationSpan sourceCodeLocationSpan, TokenKind name, bool compilerCreated = false)
        : m_source(source), m_name(name), m_sourceString(sourceString), m_sourceCodeLocationSpan(sourceCodeLocationSpan), m_compilerCreated(compilerCreated) {};
    TokenKind getTokenKind() const;
    Source* getSource() const;
    std::string_view getSourceString() const;
    std::string toString() const;
    bool operator==(const TokenKind& rhs) const;
    bool operator!=(const TokenKind& rhs) const;
    bool isCompilerCreated() const;
    SourceCodeLocationSpan getSourceCodeLocationSpan() const;
};