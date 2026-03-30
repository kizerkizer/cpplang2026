#pragma once

#include "checker/typestore.hpp"
#include "parser/node.hpp"

class TypeChecker {
private:
    std::vector<std::string>& errorMessages;
    void addErrorMessage(const std::string errorMessage);
    Type* examine(Node* node);
    TypeStore* typeStore = new TypeStore();
public:
    TypeChecker (std::vector<std::string>& errorMessages) : errorMessages(errorMessages) {};
    void typeCheck(Node* rootNode);
    TypeStore* getTypeStore();
};