#include "binder/scope.hpp"
#include "binder/name.hpp"
#include <vector>

std::string scopeKindToString(ScopeKind scopeKind) {
    switch (scopeKind) {
        case ScopeKind::Root:
            return "Root";
        case ScopeKind::Function:
            return "Function";
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

std::map<std::string, Name*> Scope::getNames() const {
    return this->names;
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

bool Scope::hasName(const std::string& nameString) const {
    return this->names.find(nameString) != this->names.end() || (this->parent != nullptr && this->parent->hasName(nameString));
}

Name* Scope::getName(const std::string& nameString) const {
    auto it = this->names.find(nameString);
    if (it != this->names.end()) {
        return it->second;
    }
    if (this->parent != nullptr) {
        return this->parent->getName(nameString);
    }
    return nullptr;
}

bool Scope::setName(Name* name) {
    auto nameString = name->getNameString();
    if (hasName(nameString)) {
        return false;
    }
    this->names[nameString] = name;
    return true;
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