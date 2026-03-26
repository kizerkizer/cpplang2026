#pragma once

#include <vector>
#include <string>
#include <map>

#include "parser/node.hpp"

class Name; // In name.hpp

enum class ScopeKind {
    Root,
    Function,
    Block,
    Class,
    Interface,
    Namespace,
    Extensional,
    Builtin,
};

std::string scopeKindToString(ScopeKind scopeKind);

class Scope {
public:
    Scope(ScopeKind kind, Scope* parent, Node* node) : node(node), kind(kind), parent(parent) {};
    ScopeKind getKind() const;
    Scope* getParent() const;
    Node* getNode() const;
    void setNode(Node* node);
    std::vector<Scope*> getChildren() const;
    void addChild(Scope* child);
    bool hasName(const std::string& nameString) const;
    Name* getName(const std::string& nameString) const;
    std::map<std::string, Name*> getNames() const;
    bool setName(Name* name);
    Scope* getThisKeywordScope() const;
    void setThisKeywordScope(Scope* thisKeywordScope);
private:
    Node* node;
    ScopeKind kind;
    Scope* parent;
    std::map<std::string, Name*> names;
    std::vector<Scope*> children;
    Scope* thisKeywordScope = nullptr;
};