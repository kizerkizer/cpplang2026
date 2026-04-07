#include <sys/syslimits.h>
#include <vector>

#include "binder/scope.hpp"
#include "binder/symbol.hpp"
#include "parser/node.hpp"

Scope* Scope::getParent() const {
    return this->m_parent;
}

std::vector<Scope*> Scope::getChildren() const {
    std::vector<Scope*> children;
    for (const auto& child : this->m_children) {
        children.push_back(child.get());
    }
    return children;
}

void Scope::addChildScope(std::unique_ptr<Scope> child) {
    this->m_children.push_back(std::move(child));
}

std::map<std::string, Symbol*> Scope::getSymbols() const {
    std::map<std::string, Symbol*> symbols;
    for (const auto& [name, symbol] : this->m_symbols) {
        symbols[name] = symbol.get();
    }
    return symbols;
}

void Scope::setNode(Node* node) {
    this->m_node = node;
}

Node* Scope::getNode() const {
    return this->m_node;
}

ScopeKind Scope::getKind() const {
    return this->m_kind;
}

Symbol* Scope::getMySymbolReference() const {
    return this->m_mySymbol;
}

void Scope::setMySymbolReference(Symbol* symbol) {
    this->m_mySymbol = symbol;
}

bool Scope::hasSymbol(const std::string& nameString) const {
    return this->getSymbol(nameString) != nullptr;
}

Symbol* Scope::getSymbol(const std::string& nameString) const {
    auto name = this->getSymbolShallow(nameString);
    if (name) {
        return name;
    } else if (this->m_parent != nullptr) {
        return this->m_parent->getSymbol(nameString);
    }
    return nullptr;
}

Symbol* Scope::getSymbolShallow(const std::string& nameString) const {
    auto it = this->m_symbols.find(nameString);
    if (it != this->m_symbols.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool Scope::hasSymbolShallow (const std::string& nameString) const {
    return this->getSymbolShallow(nameString) != nullptr;
}

bool Scope::setSymbol(std::unique_ptr<Symbol> name) {
    auto nameString = name->getNameString();
    if (this->hasSymbolShallow(nameString)) {
        return false;
    }
    this->m_symbols[nameString] = std::move(name);
    return true;
}

Scope* Scope::getFirstFunctionContainingScope() {
    if (this->m_kind == ScopeKind::Function) {
        return this;
    }
    if (this->m_parent) {
        return this->m_parent->getFirstFunctionContainingScope();
    }
    return nullptr;
}

Scope* Scope::getFirstLoopContainingScope() {
    if (this->m_kind == ScopeKind::Loop) {
        return this;
    }
    if (this->m_parent) {
        return this->m_parent->getFirstLoopContainingScope();
    }
    return nullptr;
}

Scope* Scope::getThisKeywordScope() const {
    if (this->m_thisKeywordScope) {
        return this->m_thisKeywordScope;
    }
    if (this->m_parent) {
        return this->m_parent->getThisKeywordScope();
    }
    return nullptr;
}

void Scope::setThisKeywordScope(Scope* thisKeywordScope) {
    this->m_thisKeywordScope = thisKeywordScope;
}