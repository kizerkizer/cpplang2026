#include "treewalker/value.hpp"
#include "treewalker/environment.hpp"
#include "checker/type.hpp"


// Value
Value::Value(Type* type) : m_type(type) {}

Type* Value::getType() const {
    return this->m_type;
}

bool Value::isOfType(Type* type) {
    return this->m_type->isSubtypeOf(type);
}

// ReturnValue
ReturnValue::ReturnValue(Value* value, Type* type) : Value(type), m_value(value) {}

ValueKind ReturnValue::getKind() const {
    return ValueKind::Return;
}

Value* ReturnValue::getValue() const {
    return this->m_value;
}

// BreakValue
BreakValue::BreakValue(Type* type) : Value(type) {}

ValueKind BreakValue::getKind() const {
    return ValueKind::Break;
}

// ContinueValue
ContinueValue::ContinueValue(Type* type) : Value(type) {}

ValueKind ContinueValue::getKind() const {
    return ValueKind::Continue;
}

// IntegerValue
IntegerValue::IntegerValue(long long value, Type* type) : Value(type), m_value(value) {}

ValueKind IntegerValue::getKind() const {
    return ValueKind::Integer;
}

long long IntegerValue::getValue() const {
    return this->m_value;
}

// FloatValue
FloatValue::FloatValue(double value, Type* type) : Value(type), m_value(value) {}

ValueKind FloatValue::getKind() const {
    return ValueKind::Float;
}

double FloatValue::getValue() const {
    return this->m_value;
}

// StringValue
StringValue::StringValue(const std::string_view value, Type* type) : Value(type), m_value(value) {}

ValueKind StringValue::getKind() const {
    return ValueKind::String;
}

const std::string_view StringValue::getValue() const {
    return this->m_value;
}

// BooleanValue
BooleanValue::BooleanValue(bool value, Type* type) : Value(type), m_value(value) {}

ValueKind BooleanValue::getKind() const {
    return ValueKind::Boolean;
}

bool BooleanValue::getValue() const {
    return this->m_value;
}

// VoidValue
VoidValue::VoidValue(Type* type) : Value(type) {}

ValueKind VoidValue::getKind() const {
    return ValueKind::Void;
}

std::nullptr_t VoidValue::getValue() const {
    return nullptr;
}

// EmptyValue
EmptyValue::EmptyValue(Type* type) : Value(type) {}

ValueKind EmptyValue::getKind() const {
    return ValueKind::Empty;
}

std::nullptr_t EmptyValue::getValue() const {
    return nullptr;
}

// TypeValue
TypeValue::TypeValue(Type* type) : Value(type) {}

ValueKind TypeValue::getKind() const {
    return ValueKind::Type;
}

Type* TypeValue::getValue() const {
    return this->getType();
}

// FunctionValue
FunctionValue::FunctionValue(Environment* definingEnvironment, FunctionDeclarationNode* node, FunctionType* functionType) : Value(functionType), m_definingEnvironment(definingEnvironment), m_node(node) {}

ValueKind FunctionValue::getKind() const {
    return ValueKind::Function;
}

FunctionDeclarationNode* FunctionValue::getFunctionDeclarationNode() const {
    return m_node;
}

Environment* FunctionValue::getDefiningEnvironment() const {
    return m_definingEnvironment;
}