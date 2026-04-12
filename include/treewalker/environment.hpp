#pragma once

#include <string>
#include <unordered_map>

#include "parser/node.hpp"
#include "treewalker/value.hpp"

class Environment {
private:
    std::vector<std::unique_ptr<Environment>> m_childEnvironments;
    Environment* m_parent = nullptr;
    Node* m_owningNode = nullptr;
    std::unordered_map<std::string, Value*> m_vars;
public:
    Environment(Environment* parent, Node* owningNode) : m_parent(parent), m_owningNode(owningNode) {};
    Environment* getParent() const;
    void addChildEnvironment(std::unique_ptr<Environment> childEnvironment);
    std::vector<Environment*> getChildEnvironments() const;
    std::vector<std::unique_ptr<Environment>> takeChildEnvironments();
    void setVar(const std::string& name, Value* value);
    void defineVar(const std::string& name, Value* value);
    Value* getVar(const std::string& name) const;
    Node* getOwningNode() const { return m_owningNode; };
};