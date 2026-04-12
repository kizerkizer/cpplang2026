#pragma once

#include <vector>
#include <string>
#include <map>

#include "binder/symbol.hpp"
#include "common/programuniqueid.hpp"

class Node; // In parser/parser.hpp

enum class ScopeKind {
    Root,
    Function,
    Loop,
    Block,
    If, // TODO ? maybe
    Class, // TODO
    Method, // TODO
    Interface, // TODO
    Extensional, // TODO
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
        case ScopeKind::If:
            return "If";
        case ScopeKind::Class:
            return "Class";
        case ScopeKind::Interface:
            return "Interface";
        case ScopeKind::Method:
            return "Method";
        case ScopeKind::Extensional:
            return "Extensional";
    }
}

class Scope {
private:
    int m_id;
    Node* m_node;
    ScopeKind m_kind;
    Symbol* m_mySymbol = nullptr; // Set during binding
    Scope* m_parent;
    std::map<std::string, std::unique_ptr<Symbol>> m_symbols;
    std::vector<std::unique_ptr<Scope>> m_children;
    Scope* m_thisKeywordScope = nullptr; // TODO Will be set during binding
public:
    Scope(ScopeKind kind, Scope* parent, Node* node) : m_id(getNextId()), m_node(node), m_kind(kind), m_parent(parent) {};
    ScopeKind getKind() const;
    Scope* getParent() const;
    Node* getMyNode() const; // TODO need this?
    void setMyNode(Node* node); // TODO need this?
    std::vector<Scope*> getChildren() const;
    void addChildScope(std::unique_ptr<Scope> child);
    Symbol* getMySymbolReference() const; // TODO need this?
    void setMySymbolReference(Symbol* symbol); // TODO need this?
    bool hasSymbol(const std::string& symbolString) const;
    Symbol* getSymbol(const std::string& symbolString) const;
    Symbol* getSymbolShallow(const std::string& symbolString) const;
    bool hasSymbolShallow(const std::string& symbolString) const;
    Scope* getFirstFunctionContainingScope();
    Scope* getFirstLoopContainingScope();
    std::map<std::string, Symbol*> getSymbols() const;
    bool putSymbol(std::unique_ptr<Symbol> symbol);
    Scope* getThisKeywordScope() const; // TODO
    void setThisKeywordScope(Scope* thisKeywordScope); // TODO
};