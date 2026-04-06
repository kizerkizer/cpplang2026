#include "lexer/token.hpp"
#include "common/sourcecodelocation.hpp"

// Token
TokenKind Token::getTokenKind() const {
    return this->name;
}

bool Token::operator==(const TokenKind& rhs) const {
    return this->name == rhs;
}

bool Token::operator!=(const TokenKind& rhs) const {
    return this->name != rhs;
}

SourceCodeLocationSpan Token::getSourceCodeLocationSpan() const {
    return this->sourceCodeLocationSpan;
}

std::string_view Token::getSourceString() const {
    return this->sourceString;
}

std::string Token::toString() const {
    return "<" + std::string(tokenKindToString(this->name)) + "> ('" + std::string(this->sourceString) + "')";
}

bool Token::isCompilerCreated() const {
    return this->compilerCreated;
}