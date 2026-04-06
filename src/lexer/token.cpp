#include "lexer/token.hpp"
#include "common/sourcecodelocation.hpp"

std::string tokenKindToString(const TokenKind &tokenKind) {
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

int getPrecedence(const TokenKind &tokenName) {
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

bool getAssociativity(const TokenKind &tokenName) {
    switch (tokenName) {
        case TokenKind::Not:
        case TokenKind::Equal:
        case TokenKind::AsteriskAsterisk:
            return false; // right associative
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
            return true; // left associative
        default: return true; // default to left associative
    }
}

TokenKind Token::getTokenKind() const {
    return this->name;
}

bool Token::operator==(const TokenKind& rhs) const {
    return this->name == rhs;
}

bool Token::operator!=(const TokenKind& rhs) const {
    return this->name != rhs;
}

/*int Token::getFirstIndex() const {
    return this->firstIndex;
}

int Token::getFirstLine() const {
    return this->firstLine;
}

int Token::getFirstColumn() const {
    return this->firstColumn;
}

int Token::getLastIndex() const {
    return this->lastIndex;
}

int Token::getLastLine() const {
    return this->lastLine;
}

int Token::getLastColumn() const {
    return this->lastColumn;
}*/

/*void Token::setFirstSourceCodeLocation(SourceCodeLocation location) {
    std::tie(this->firstIndex, this->firstLine, this->firstColumn) = location;
}

void Token::setLastSourceCodeLocation(SourceCodeLocation location) {
    std::tie(this->lastIndex, this->lastLine, this->lastColumn) = location;
}*/

/*SourceCodeLocation Token::getFirstSourceCodeLocation() const {
    return {this->firstIndex, this->firstLine, this->firstColumn};
}

SourceCodeLocation Token::getLastSourceCodeLocation() const {
    // TODO set line and column appropriately for multi-line tokens
    return {this->firstIndex + this->getSourceString().size(), this->firstLine, this->firstColumn + this->getSourceString().size()};
}*/

SourceCodeLocationSpan Token::getSourceCodeLocationSpan() const {
    return this->sourceCodeLocationSpan;
}

std::string_view Token::getSourceString() const {
    return this->sourceString;
}

std::string Token::toString() const {
    return "<" + tokenKindToString(this->name) + "> ('" + std::string(this->sourceString) + "')";
}

bool Token::isCompilerCreated() const {
    return this->compilerCreated;
}