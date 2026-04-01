#include "binder/symbol.hpp"
#include "binder/scope.hpp"

std::string symbolKindToString(SymbolKind symbolKind) {
    switch (symbolKind) {
        case SymbolKind::Variable:
            return "Variable";
        case SymbolKind::Function:
            return "Function";
        case SymbolKind::Loop:
            return "Loop";
        case SymbolKind::Parameter:
            return "Parameter";
        case SymbolKind::Type:
            return "Type";
        case SymbolKind::TypeParameter:
            return "TypeParameter";
        case SymbolKind::Class:
            return "Class";
        case SymbolKind::ClassMember:
            return "ClassMember";
        case SymbolKind::Interface:
            return "Interface";
        case SymbolKind::InterfaceMember:
            return "InterfaceMember";
        case SymbolKind::ExtensionalMember:
            return "ExtensionalMember";
        case SymbolKind::NamespaceMember:
            return "NamespaceMember";
        case SymbolKind::Builtin:
            return "Builtin";
    }
}

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