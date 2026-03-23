#include "token.hpp"

std::string tokenNameToString(const TokenName &name) {
    switch (name) {
        using enum TokenName;
        case OutOfRange: return "OutOfRange";
        case TriviaWhitespace: return "Whitespace";
        case TriviaCommentShort: return "CommentShort";
        case TriviaCommentLong: return "CommentLong";
        case Identifier: return "Identifier";
        case IntegerLiteral: return "IntegerLiteral";
        case StringLiteral: return "StringLiteral";
        case Dot: return "Dot";
        case Comma: return "Comma";
        case Semicolon: return "Semicolon";
        case ParenthesisOpen: return "ParenthesisOpen";
        case ParenthesisClose: return "ParenthesisClose";
        case BraceOpen: return "BraceOpen";
        case BraceClose: return "BraceClose";
        case BracketOpen: return "BracketOpen";
        case BracketClose: return "BracketClose";
        case Plus: return "Plus";
        case Minus: return "Minus";
        case Asterisk: return "Asterisk";
        case Slash: return "Slash";
        case Equal: return "Equal";
        case LessThan: return "LessThan";
        case GreaterThan: return "GreaterThan";
        case EqualEqual: return "EqualEqual";
        case NotEqual: return "NotEqual";
        case LessThanEqual: return "LessThanEqual";
        case GreaterThanEqual: return "GreaterThanEqual";
        case And: return "And";
        case Or: return "Or";
        case Not: return "Not";
        case KeywordIf: return "KeywordIf";
        case KeywordElse: return "KeywordElse";
        case KeywordClass: return "KeywordClass";
        case KeywordWhile: return "KeywordWhile";
        case KeywordFunction: return "KeywordFunction";
        case KeywordReturn: return "KeywordReturn";
        case KeywordContinue: return "KeywordContinue";
        case KeywordBreak: return "KeywordBreak";
        case KeywordVar: return "KeywordVar";
    }
}

TokenName Token::getTokenName() const {
    return this->name;
}

bool Token::operator==(const TokenName& rhs) const {
    return this->name == rhs;
}

bool Token::operator!=(const TokenName& rhs) const {
    return this->name != rhs;
}

bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.name == rhs.name && lhs.sourceString == rhs.sourceString && lhs.index == rhs.index && lhs.line == rhs.line && lhs.column == rhs.column;
}

bool operator!=(const Token& lhs, const Token& rhs) {
    return !(lhs == rhs);
}

int Token::getIndex() const {
    return this->index;
}

int Token::getLine() const {
    return this->line;
}

int Token::getColumn() const {
    return this->column;
}

std::string Token::getSourceString() const {
    return this->sourceString;
}

std::string Token::toString() const {
    return "<" + tokenNameToString(this->name) + "> ('" + this->sourceString + "')";
}