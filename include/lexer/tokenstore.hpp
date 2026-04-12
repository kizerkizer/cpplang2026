#pragma once

#include <unordered_map>

#include "lexer/token.hpp"

class TokenStore {
private:
    std::unordered_map<int, std::unique_ptr<Token>> m_tokens;
public:
    Token* storeToken(std::unique_ptr<Token> token);
    const Token* getTokenById(int tokenId) const;
};