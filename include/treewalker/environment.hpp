#pragma once

#include <string>
#include <unordered_map>

#include "parser/node.hpp"
#include "treewalker/value.hpp"

class Environment {
private:
    std::vector<std::unique_ptr<Environment>> m_childEnvironments;
    Environment* m_parent = nullptr;
    Node* m_owningNode = nullptr;
    std::unordered_map<std::string, Value*> m_vars;
public:
    Environment(Environment* parent, Node* owningNode) : m_parent(parent), m_owningNode(owningNode) {};
    Environment* getParent() const;
    void addChildEnvironment(std::unique_ptr<Environment> childEnvironment);
    std::vector<Environment*> getChildEnvironments() const;
    std::vector<std::unique_ptr<Environment>> takeChildEnvironments();
    void setVar(const std::string& name, Value* value);
    void defineVar(const std::string& name, Value* value);
    Value* getVar(const std::string& name) const;
    Node* getOwningNode() const { return m_owningNode; };
};

class FunctionValue : public Value { // TODO
private:
    Node* m_functionDeclarationNode;
    Environment* m_definingEnvironment;
public:
    FunctionValue(Node* functionDeclarationNode, Environment* definingEnvironment) : m_functionDeclarationNode(functionDeclarationNode), m_definingEnvironment(definingEnvironment) {};
    ValueKind getKind() const override { return ValueKind::Function; };
    Node* getValue() const { return m_functionDeclarationNode; };
    Node* getNode() const { return m_functionDeclarationNode; };
    Environment* getDefiningEnvironment() const { return m_definingEnvironment; };
};

class ValueStore {
private:
    std::vector<std::unique_ptr<Value>> m_values;
public:
    void addValue(std::unique_ptr<Value> value);
    IntegerValue* makeIntegerValue(int value);
    FloatValue* makeFloatValue(double value);
    StringValue* makeStringValue(const std::string& value);
    BooleanValue* makeBooleanValue(bool value);
    EmptyValue* makeEmptyValue();
    VoidValue* makeVoidValue();
    FunctionValue* makeFunctionValue(Node* functionDeclarationNode, Environment* definingEnvironment);
    ReturnValue* makeReturnValue(Value* value);
    BreakValue* makeBreakValue();
    ContinueValue* makeContinueValue();
};
