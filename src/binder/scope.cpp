#include <sys/syslimits.h>
#include <vector>

#include "binder/scope.hpp"
#include "binder/symbol.hpp"
#include "parser/node.hpp"

std::string scopeKindToString(ScopeKind scopeKind) {
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

Scope* Scope::getParent() const {
    return this->parent;
}

std::vector<Scope*> Scope::getChildren() const {
    return this->children;
}

void Scope::addChild(Scope *child) {
    this->children.push_back(child);
}

std::map<std::string, Symbol*> Scope::getSymbols() const {
    return this->symbols;
}

void Scope::setNode(Node* node) {
    this->node = node;
}

Node* Scope::getNode() const {
    return this->node;
}

ScopeKind Scope::getKind() const {
    return this->kind;
}

Symbol* Scope::getMySymbolReference() const {
    return this->mySymbol;
}

void Scope::setMySymbolReference(Symbol* symbol) {
    this->mySymbol = symbol;
}

bool Scope::hasSymbol(const std::string& nameString) const {
    return this->getSymbol(nameString) != nullptr;
}

Symbol* Scope::getSymbol(const std::string& nameString) const {
    auto name = this->getSymbolShallow(nameString);
    if (name) {
        return name;
    } else if (this->parent != nullptr) {
        return this->parent->getSymbol(nameString);
    }
    return nullptr;
}

Symbol* Scope::getSymbolShallow(const std::string& nameString) const {
    auto it = this->symbols.find(nameString);
    if (it != this->symbols.end()) {
        return it->second;
    }
    return nullptr;
}

bool Scope::hasSymbolShallow (const std::string& nameString) const {
    return this->getSymbolShallow(nameString) != nullptr;
}

bool Scope::setSymbol(Symbol* name) {
    auto nameString = name->getNameString();
    if (this->hasSymbolShallow(nameString)) {
        return false;
    }
    this->symbols[nameString] = name;
    return true;
}

Scope* Scope::getFirstFunctionContainingScope() {
    if (this->kind == ScopeKind::Function) {
        return this;
    }
    if (this->parent) {
        return this->parent->getFirstFunctionContainingScope();
    }
    return nullptr;
}

Scope* Scope::getFirstLoopContainingScope() {
    if (this->kind == ScopeKind::Loop) {
        return this;
    }
    if (this->parent) {
        return this->parent->getFirstLoopContainingScope();
    }
    return nullptr;
}

Scope* Scope::getThisKeywordScope() const {
    if (this->thisKeywordScope) {
        return this->thisKeywordScope;
    }
    if (this->parent) {
        return this->parent->getThisKeywordScope();
    }
    return nullptr;
}

void Scope::setThisKeywordScope(Scope* thisKeywordScope) {
    this->thisKeywordScope = thisKeywordScope;
}