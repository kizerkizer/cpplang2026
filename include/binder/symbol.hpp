#pragma once

#include "flowbuilder/flowgraph.hpp"
#include "parser/node.hpp"
#include <string>

class Scope; // In scope.hpp

enum class SymbolKind {
    Variable,
    Function,
    Parameter,
    Loop,
    Type,
    TypeParameter,
    Class,
    ClassMember,
    Interface,
    InterfaceMember,
    ExtensionalMember,
    NamespaceMember,
    Builtin,
};

constexpr const char* symbolKindToString(SymbolKind symbolKind) {
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

enum class SymbolModifierFlags : uint8_t {
    None = 0,
    Public = 1 << 0,
    Private = 1 << 1,
    Protected = 1 << 2,
    Static = 1 << 3,
    Abstract = 1 << 4,
};

inline SymbolModifierFlags operator|(SymbolModifierFlags a, SymbolModifierFlags b) {
    return static_cast<SymbolModifierFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

class Symbol {
public:
    Symbol(Scope* containingScope, Node* node, SymbolKind kind, SymbolModifierFlags modifierFlags, const std::string& nameString) : nameString(nameString), containingScope(containingScope), node(node), kind(kind), modifierFlags(modifierFlags) {};
    const std::string& getNameString() const;
    const Scope* getContainingScope() const;
    SymbolKind getKind() const;
    Node* getDefiningNode() const;
    SymbolModifierFlags getModifierFlags() const;
    Type* getType();
    void setType(Type* type);
    FlowGraph* getFlowGraph();
    void setFlowGraph(FlowGraph* flowGraph);
private:
    std::string nameString;
    Scope* containingScope;
    Node* node;
    SymbolKind kind;
    SymbolModifierFlags modifierFlags;
    Type* type = nullptr; // set in type checking
    FlowGraph* flowGraph = nullptr; // set in flowbuilder TODO ?
};
