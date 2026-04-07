#pragma once

#include <string_view>

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
    }
}

class Value {
public:
    virtual ~Value() = default;
    virtual ValueKind getKind() const = 0;
};

class ReturnValue : public Value {
private:
    Value* m_value;
public:
    ReturnValue(Value* value) : m_value(value) {};
    ValueKind getKind() const override { return ValueKind::Return; };
    Value* getValue() const { return m_value; };
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
private:
    int m_value;
public:
    IntegerValue(int value) { m_value = value; };
    ValueKind getKind() const override { return ValueKind::Integer; };
    int getValue() const { return m_value; };
};

class FloatValue : public Value {
private:
    double m_value;
public:
    FloatValue(double value) { m_value = value; };
    ValueKind getKind() const override { return ValueKind::Float; };
    double getValue() const { return m_value; };
};

class StringValue : public Value {
private:
    std::string_view m_value;
public:
    StringValue(const std::string_view value) { m_value = value; };
    ValueKind getKind() const override { return ValueKind::String; };
    const std::string_view getValue() const { return m_value; };
};

class BooleanValue : public Value {
private:
    bool m_value;
public:
    BooleanValue(bool value) { m_value = value; };
    ValueKind getKind() const override { return ValueKind::Boolean; };
    bool getValue() const { return m_value; };
};

class EmptyValue : public Value {
public:
    ValueKind getKind() const override { return ValueKind::Empty; };
    std::nullptr_t getValue() const { return nullptr; }
};

class VoidValue : public Value {
public:
    ValueKind getKind() const override { return ValueKind::Void; };
    std::nullptr_t getValue() const { return nullptr; };
};