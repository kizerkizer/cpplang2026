#pragma once

#include <string_view>

#include "checker/type.hpp"
#include "parser/node.hpp"

class Environment; // defined in environment.hpp

enum class ValueKind {
    Integer,
    Float,
    String,
    Boolean,
    Empty,
    Void,
    Function,
    Instance,
    // TODO the following should be in part of separate structure
    Return,
    Break,
    Continue,
    Type,
};

constexpr const char* valueKindToString(ValueKind valueKind) {
    switch (valueKind) {
        case ValueKind::Integer:
            return "Integer";
        case ValueKind::Float:
            return "Float";
        case ValueKind::String:
            return "String";
        case ValueKind::Boolean:
            return "Boolean";
        case ValueKind::Empty:
            return "Empty";
        case ValueKind::Void:
            return "Void";
        case ValueKind::Function:
            return "Function";
        case ValueKind::Instance:
            return "Instance";
        case ValueKind::Return:
            return "Return";
        case ValueKind::Break:
            return "Break";
        case ValueKind::Continue:
            return "Continue";
        case ValueKind::Type:
            return "Type";
    }
}

class Value {
private:
    Type* m_type;
public:
    Value(Type* type);
    virtual ~Value() = default;
    virtual ValueKind getKind() const = 0;
    bool isOfType(Type* type);
    Type* getType() const;
};

class ReturnValue : public Value {
private:
    Value* m_value;
public:
    ReturnValue(Value* value, Type* type);
    ValueKind getKind() const override;
    Value* getValue() const;
};

class BreakValue : public Value {
public:
    BreakValue(Type* type);
    ValueKind getKind() const override;
};

class ContinueValue : public Value {
public:
    ContinueValue(Type* type);
    ValueKind getKind() const override;
};

class IntegerValue : public Value {
private:
    long long m_value;
public:
    IntegerValue(long long value, Type* type);
    ValueKind getKind() const override;
    long long getValue() const;
};

class FloatValue : public Value {
private:
    double m_value;
public:
    FloatValue(double value, Type* type);
    ValueKind getKind() const override;
    double getValue() const;
};

class StringValue : public Value {
private:
    std::string_view m_value;
public:
    StringValue(const std::string_view value, Type* type);
    ValueKind getKind() const override;
    const std::string_view getValue() const;
};

class BooleanValue : public Value {
private:
    bool m_value;
public:
    BooleanValue(bool value, Type* type);
    ValueKind getKind() const override;
    bool getValue() const;
};

class EmptyValue : public Value {
public:
    EmptyValue(Type* type);
    ValueKind getKind() const override;
    std::nullptr_t getValue() const;
};

class VoidValue : public Value {
public:
    VoidValue(Type* type);
    ValueKind getKind() const override;
    std::nullptr_t getValue() const;
};

class TypeValue : public Value {
public:
    TypeValue(Type* type);
    ValueKind getKind() const override;
    Type* getValue() const;
};

class FunctionValue : public Value { // TODO
private:
    Environment* m_definingEnvironment;
    FunctionDeclarationNode* m_node;
public:
    FunctionValue(Environment* definingEnvironment, FunctionDeclarationNode* node, FunctionType* functionType);
    ValueKind getKind() const override;
    //FunctionType* getValue() const;
    FunctionDeclarationNode* getFunctionDeclarationNode() const;
    Environment* getDefiningEnvironment() const;
};
