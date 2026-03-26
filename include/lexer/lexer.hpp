#pragma once

#include <string>
#include <vector>
#include "common/sourcecodelocation.hpp"
#include "token.hpp"

class Lexer {
public:
    Lexer();
    std::vector<Token> lex(const std::string& sourceString, std::vector<std::string>& errorMessages_out);
private:
    std::string sourceString;
    size_t index = 0;
    size_t line = 1;
    size_t column = 1;
    void advance(const int &steps);
    SourceCodeLocation getCurrentSourceCodeLocation() const;
    char getCharacter (const int &offset) const;
    bool isPastSourceStringEnd () const;
    std::string makeErrorMessage(const std::string& message) const;
};