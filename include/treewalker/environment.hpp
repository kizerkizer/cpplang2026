#pragma once

#include <string>
#include <unordered_map>

#include "parser/node.hpp"

enum class ValueKind {
    Integer,
    Float,
    String,
    Boolean,
    Empty,
    Void,
    Function,
    Instance,
    Return,
    Break,
    Continue,
};

#define valueKindToString(valueKind) \
    (valueKind == ValueKind::Integer) ? "Integer" : \
    (valueKind == ValueKind::Float) ? "Float" : \
    (valueKind == ValueKind::String) ? "String" : \
    (valueKind == ValueKind::Boolean) ? "Boolean" : \
    (valueKind == ValueKind::Empty) ? "Empty" : \
    (valueKind == ValueKind::Void) ? "Void" : \
    (valueKind == ValueKind::Function) ? "Function" : \
    (valueKind == ValueKind::Instance) ? "Instance" : \
    (valueKind == ValueKind::Return) ? "Return" : \
    (valueKind == ValueKind::Break) ? "Break" : \
    (valueKind == ValueKind::Continue) ? "Continue" : \
    "Unknown"

class Value {
public:
    virtual ~Value() = default;
    virtual ValueKind getKind() const = 0;
};

class ReturnValue : public Value {
public:
    ReturnValue(Value* value) : value(value) {};
    ValueKind getKind() const override { return ValueKind::Return; };
    Value* getValue() const { return value; };
private:
    Value* value;
};

class BreakValue : public Value {
public:
    BreakValue() {};
    ValueKind getKind() const override { return ValueKind::Break; };
};

class ContinueValue : public Value {
public:
    ContinueValue() {};
    ValueKind getKind() const override { return ValueKind::Continue; };
};

class IntegerValue : public Value {
public:
    IntegerValue(int value) { this->value = value; };
    ValueKind getKind() const override { return ValueKind::Integer; };
    int getValue() const { return value; };
private:
    int value;
};

class FloatValue : public Value {
public:
    FloatValue(double value) { this->value = value; };
    ValueKind getKind() const override { return ValueKind::Float; };
    double getValue() const { return value; };
private:
    double value;
};

class StringValue : public Value {
public:
    StringValue(const std::string& value) { this->value = value; };
    ValueKind getKind() const override { return ValueKind::String; };
    const std::string& getValue() const { return value; };
private:
    std::string value;
};

class BooleanValue : public Value {
public:
    BooleanValue(bool value) { this->value = value; };
    ValueKind getKind() const override { return ValueKind::Boolean; };
    bool getValue() const { return value; };
private:
    bool value;
};

class EmptyValue : public Value {
public:
    EmptyValue() { this->value = nullptr; };
    ValueKind getKind() const override { return ValueKind::Empty; };
    std::nullptr_t getValue() const { return value; };
private:
    std::nullptr_t value;
};

class VoidValue : public Value {
public:
    VoidValue() { this->value = nullptr; };
    ValueKind getKind() const override { return ValueKind::Void; };
    std::nullptr_t getValue() const { return value; };
private:
    std::nullptr_t value;
};

class Environment {
public:
    Environment(Environment* parent, Node* owningNode) : parent(parent), owningNode(owningNode) {};
    Environment* getParent() const;
    void addChildEnvironment(std::unique_ptr<Environment> childEnvironment);
    std::vector<Environment*> getChildEnvironments() const;
    std::vector<std::unique_ptr<Environment>> takeChildEnvironments();
    void setVar(const std::string& name, Value* value);
    void defineVar(const std::string& name, Value* value);
    Value* getVar(const std::string& name) const;
    Node* getOwningNode() const { return owningNode; };
private:
    std::vector<std::unique_ptr<Environment>> childEnvironments;
    Environment* parent = nullptr;
    Node* owningNode = nullptr;
    std::unordered_map<std::string, Value*> vars;
};

class FunctionValue : public Value { // TODO
public:
    FunctionValue(Node* functionDeclarationNode, Environment* definingEnvironment) : functionDeclarationNode(functionDeclarationNode), definingEnvironment(definingEnvironment) {};
    ValueKind getKind() const override { return ValueKind::Function; };
    Node* getValue() const { return functionDeclarationNode; };
    Node* getNode() const { return functionDeclarationNode; };
    Environment* getDefiningEnvironment() const { return definingEnvironment; };
private:
    Node* functionDeclarationNode;
    Environment* definingEnvironment;
};

class ValueStore {
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
private:
    std::vector<std::unique_ptr<Value>> values;
};
