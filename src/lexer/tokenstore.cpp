#include "lexer/tokenstore.hpp"

Token* TokenStore::storeToken(std::unique_ptr<Token> token) {
    int tokenId = token->getId();
    m_tokens[tokenId] = std::move(token);
    return m_tokens[tokenId].get();
}

const Token* TokenStore::getTokenById(int tokenId) const {
    auto it = m_tokens.find(tokenId);
    if (it != m_tokens.end()) {
        return it->second.get();
    }
    return nullptr;
}