#pragma once

#include <string>
#include <vector>
#include "token.hpp"

class Lexer {
public:
    Lexer();
    std::vector<Token> lex (const std::string& sourceString, std::vector<std::string>& errorMessages_out);
private:
    std::string sourceString;
    size_t index = 0;
    int line = 1;
    int column = 1;
    void advance(const int &steps);
    std::tuple<size_t, size_t, size_t> getCounters() const;
    char getCharacter (const int &offset) const;
    bool isPastSourceStringEnd () const;
    std::string makeErrorMessage(const std::string& message) const;
};