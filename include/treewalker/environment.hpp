#pragma once

#include <string>
#include <unordered_map>

#include "parser/node.hpp"

class Value {
public:
    virtual ~Value() = default;
};

class Environment {
public:
    Environment(Environment* parent, Node* owningNode) : parent(parent), owningNode(owningNode) {};
    Environment* getParent() const;
    bool setVar(const std::string& name, Value* value);
    Value* getVar(const std::string& name) const;
private:
    Environment* parent = nullptr;
    Node* owningNode = nullptr;
    std::unordered_map<std::string, Value*> vars;
};