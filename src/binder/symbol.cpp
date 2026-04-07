#include "binder/symbol.hpp"
#include "binder/scope.hpp"

// Symbol
const std::string& Symbol::getNameString() const {
    return this->m_nameString;
}

const Scope* Symbol::getContainingScope() const {
    return this->m_containingScope;
}

SymbolKind Symbol::getKind() const {
    return this->m_kind;
}

Node* Symbol::getDefiningNode() const {
    return this->m_node;
}

FlowGraph* Symbol::getFlowGraph() {
    return this->m_flowGraph;
}

void Symbol::setFlowGraph(FlowGraph* flowGraph) {
    this->m_flowGraph = flowGraph;
}

Type* Symbol::getType() {
    return this->m_type;
}

void Symbol::setType(Type* type) {
    this->m_type = type;
}