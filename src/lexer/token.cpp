#include "lexer/token.hpp"
#include "common/sourcecodelocation.hpp"

// Token
TokenKind Token::getTokenKind() const {
    return m_tokenKind;
}

bool Token::operator==(const TokenKind& rhs) const {
    return m_tokenKind == rhs;
}

bool Token::operator!=(const TokenKind& rhs) const {
    return m_tokenKind != rhs;
}

int Token::getId() const {
    return m_id;
}

bool Token::isKeyword() const {
    return IS_TOKENKIND_KEYWORD(m_tokenKind);
}

bool Token::isLiteral() const {
    return IS_TOKENKIND_LITERAL(m_tokenKind);
}

bool Token::isTypePrimitve() const {
    return m_tokenKind == TokenKind::TypePrimitiveBoolean || m_tokenKind == TokenKind::TypePrimitiveEmpty || m_tokenKind == TokenKind::TypePrimitiveFloat || m_tokenKind == TokenKind::TypePrimitiveInteger || m_tokenKind == TokenKind::TypePrimitiveString || m_tokenKind == TokenKind::TypePrimitiveAny || m_tokenKind == TokenKind::TypePrimitiveVoid;
}

SourceCodeLocationSpan Token::getSourceCodeLocationSpan() const {
    return m_sourceCodeLocationSpan;
}

std::string_view Token::getSourceString() const {
    return m_sourceString;
}

std::string Token::toString() const {
    return "<" + std::string(tokenKindToString(m_tokenKind)) + "> ('" + std::string(m_sourceString) + "')";
}

bool Token::isCompilerCreated() const {
    return m_compilerCreated;
}