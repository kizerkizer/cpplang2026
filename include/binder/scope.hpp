#pragma once

#include <vector>
#include <string>
#include <map>

class Node; // In parser/parser.hpp
class Symbol; // In symbol.hpp

enum class ScopeKind {
    Root,
    Function,
    Loop,
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
    Symbol* getMySymbolReference() const;
    void setMySymbolReference(Symbol* symbol);
    bool hasSymbol(const std::string& symbolString) const;
    Symbol* getSymbol(const std::string& symbolString) const;
    Symbol* getSymbolShallow(const std::string& symbolString) const;
    bool hasSymbolShallow(const std::string& symbolString) const;
    Scope* getFirstFunctionContainingScope();
    Scope* getFirstLoopContainingScope();
    std::map<std::string, Symbol*> getSymbols() const;
    bool setSymbol(Symbol* symbol);
    Scope* getThisKeywordScope() const;
    void setThisKeywordScope(Scope* thisKeywordScope);
private:
    Node* node;
    ScopeKind kind;
    Symbol* mySymbol = nullptr; // Set during binding
    Scope* parent;
    std::map<std::string, Symbol*> symbols;
    std::vector<Scope*> children;
    Scope* thisKeywordScope = nullptr; // TODO Set during binding
};