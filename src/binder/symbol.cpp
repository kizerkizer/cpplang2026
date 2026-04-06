#include "binder/symbol.hpp"
#include "binder/scope.hpp"

// Symbol
const std::string& Symbol::getNameString() const {
    return this->nameString;
}

const Scope* Symbol::getContainingScope() const {
    return this->containingScope;
}

SymbolKind Symbol::getKind() const {
    return this->kind;
}

Node* Symbol::getDefiningNode() const {
    return this->node;
}

FlowGraph* Symbol::getFlowGraph() {
    return this->flowGraph;
}

void Symbol::setFlowGraph(FlowGraph* flowGraph) {
    this->flowGraph = flowGraph;
}

Type* Symbol::getType() {
    return this->type;
}

void Symbol::setType(Type* type) {
    this->type = type;
}