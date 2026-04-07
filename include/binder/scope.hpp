#pragma once

#include <vector>
#include <string>
#include <map>

#include "binder/symbol.hpp"

class Node; // In parser/parser.hpp

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

constexpr const char* scopeKindToString(ScopeKind scopeKind) {
    switch (scopeKind) {
        case ScopeKind::Root:
            return "Root";
        case ScopeKind::Function:
            return "Function";
        case ScopeKind::Loop:
            return "Loop";
        case ScopeKind::Block:
            return "Block";
        case ScopeKind::Class:
            return "Class";
        case ScopeKind::Interface:
            return "Interface";
        case ScopeKind::Namespace:
            return "Namespace";
        case ScopeKind::Extensional:
            return "Extensional";
        case ScopeKind::Builtin:
            return "Builtin";
    }
}

class Scope {
private:
    Node* m_node;
    ScopeKind m_kind;
    Symbol* m_mySymbol = nullptr; // Set during binding
    Scope* m_parent;
    std::map<std::string, std::unique_ptr<Symbol>> m_symbols;
    std::vector<std::unique_ptr<Scope>> m_children;
    Scope* m_thisKeywordScope = nullptr; // TODO Set during binding
public:
    Scope(ScopeKind kind, Scope* parent, Node* node) : m_node(node), m_kind(kind), m_parent(parent) {};
    ScopeKind getKind() const;
    Scope* getParent() const;
    Node* getNode() const;
    void setNode(Node* node);
    std::vector<Scope*> getChildren() const;
    void addChildScope(std::unique_ptr<Scope> child);
    Symbol* getMySymbolReference() const;
    void setMySymbolReference(Symbol* symbol);
    bool hasSymbol(const std::string& symbolString) const;
    Symbol* getSymbol(const std::string& symbolString) const;
    Symbol* getSymbolShallow(const std::string& symbolString) const;
    bool hasSymbolShallow(const std::string& symbolString) const;
    Scope* getFirstFunctionContainingScope();
    Scope* getFirstLoopContainingScope();
    std::map<std::string, Symbol*> getSymbols() const;
    bool setSymbol(std::unique_ptr<Symbol> symbol);
    Scope* getThisKeywordScope() const;
    void setThisKeywordScope(Scope* thisKeywordScope);
};