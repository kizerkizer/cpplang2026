#include "lexer/token.hpp"
#include "common/sourcecodelocation.hpp"

// Token
TokenKind Token::getTokenKind() const {
    return m_name;
}

bool Token::operator==(const TokenKind& rhs) const {
    return m_name == rhs;
}

bool Token::operator!=(const TokenKind& rhs) const {
    return m_name != rhs;
}

int Token::getId() const {
    return m_id;
}

bool Token::isKeyword() const {
    return IS_TOKENKIND_KEYWORD(m_name);
}

bool Token::isLiteral() const {
    return IS_TOKENKIND_LITERAL(m_name);
}

bool Token::isTypePrimitve() const {
    return m_name == TokenKind::TypePrimitiveBoolean || m_name == TokenKind::TypePrimitiveEmpty || m_name == TokenKind::TypePrimitiveFloat || m_name == TokenKind::TypePrimitiveInteger || m_name == TokenKind::TypePrimitiveString || m_name == TokenKind::TypePrimitiveAny || m_name == TokenKind::TypePrimitiveVoid;
}

SourceCodeLocationSpan Token::getSourceCodeLocationSpan() const {
    return m_sourceCodeLocationSpan;
}

std::string_view Token::getSourceString() const {
    return m_sourceString;
}

std::string Token::toString() const {
    return "<" + std::string(tokenKindToString(m_name)) + "> ('" + std::string(m_sourceString) + "')";
}

bool Token::isCompilerCreated() const {
    return m_compilerCreated;
}