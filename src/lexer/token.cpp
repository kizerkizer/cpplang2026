#include "lexer/token.hpp"
#include "common/sourcecodelocation.hpp"

// Token
TokenKind Token::getTokenKind() const {
    return this->m_name;
}

bool Token::operator==(const TokenKind& rhs) const {
    return this->m_name == rhs;
}

bool Token::operator!=(const TokenKind& rhs) const {
    return this->m_name != rhs;
}

SourceCodeLocationSpan Token::getSourceCodeLocationSpan() const {
    return this->m_sourceCodeLocationSpan;
}

std::string_view Token::getSourceString() const {
    return this->m_sourceString;
}

std::string Token::toString() const {
    return "<" + std::string(tokenKindToString(this->m_name)) + "> ('" + std::string(this->m_sourceString) + "')";
}

bool Token::isCompilerCreated() const {
    return this->m_compilerCreated;
}