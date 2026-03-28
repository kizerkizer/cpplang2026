#pragma once

#include "binder/scope.hpp"
#include "flowbuilder/flowgraph.hpp"
#include "parser/node.hpp"
#include <memory>
#include <string>

enum class NameKind {
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

std::string nameKindToString(NameKind nameKind);

enum class NameModifierFlags : uint8_t {
    None = 0,
    Public = 1 << 0,
    Private = 1 << 1,
    Protected = 1 << 2,
    Static = 1 << 3,
    Abstract = 1 << 4,
};

inline NameModifierFlags operator|(NameModifierFlags a, NameModifierFlags b) {
    return static_cast<NameModifierFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

class Name {
public:
    Name(Scope* containingScope, Node* node, NameKind kind, NameModifierFlags modifierFlags, const std::string& nameString) : nameString(nameString), containingScope(std::move(containingScope)), node(node), kind(kind), modifierFlags(modifierFlags) {};
    const std::string& getNameString() const;
    const Scope* getContainingScope() const;
    NameKind getKind() const;
    const Node* getNode() const;
    NameModifierFlags getModifierFlags() const;
    Type* getType();
    void setType(std::unique_ptr<Type> type); // TODO maybe change to Type*
    FlowGraph* getFlowGraph();
    void setFlowGraph(FlowGraph* flowGraph);
private:
    std::string nameString;
    Scope* containingScope;
    Node* node;
    NameKind kind;
    NameModifierFlags modifierFlags;
    std::unique_ptr<Type> type = nullptr; // set in type checking
    FlowGraph* flowGraph = nullptr; // set in flowbuilder
};