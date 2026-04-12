#include "checker/type.hpp"
#include "binder/symbol.hpp"

//Type
TypeKind Type::getTypeKind() const {
    return m_kind;
}

TypeFlags Type::getTypeFlags() const {
    return m_flags;
}

void Type::setTypeFlags(TypeFlags flags) {
    m_flags = flags;
}

void Type::setTypeFlag(TypeFlags flag) {
    m_flags = m_flags | flag;
}

bool Type::isPrimitive() const {
    return this->getTypeKind() == TypeKind::Primitive;
}

bool Type::isError() const {
    return this->getTypeKind() == TypeKind::Error;
}

bool Type::isUnion() const {
    return this->getTypeKind() == TypeKind::Union;
}

bool Type::isVoid() const {
    return this->getTypeKind() == TypeKind::Void;
}

bool Type::isAny() const {
    return this->getTypeKind() == TypeKind::Any;
}

bool Type::isFunction() const {
    return this->getTypeKind() == TypeKind::Function;
}

bool Type::isBoolean() const {
    if (this->isPrimitive()) {
        auto primitiveType = static_cast<const PrimitiveType*>(this);
        return primitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Boolean;
    }
    return false;
}

bool Type::isFloat() const {
    if (this->isPrimitive()) {
        auto primitiveType = static_cast<const PrimitiveType*>(this);
        return primitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Float;
    }
    return false;
}

bool Type::isInteger() const {
    if (this->isPrimitive()) {
        auto primitiveType = static_cast<const PrimitiveType*>(this);
        return primitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Integer;
    }
    return false;
}

bool Type::isString() const {
    if (this->isPrimitive()) {
        auto primitiveType = static_cast<const PrimitiveType*>(this);
        return primitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::String;
    }
    return false;
}

bool Type::isEmpty() const {
    if (this->isPrimitive()) {
        auto primitiveType = static_cast<const PrimitiveType*>(this);
        return primitiveType->getPrimitiveTypeKind() == PrimitiveTypeKind::Empty;
    }
    return false;
}

bool Type::isTypeType() const {
    return this->getTypeKind() == TypeKind::TypeType;
}

int Type::getId() const {
    return m_id;
}

bool Type::operator==(const Type& rhs) const {
    return this->isSubtypeOf(const_cast<Type*>(&rhs)) && const_cast<Type*>(&rhs)->isSubtypeOf(const_cast<Type*>(this));
}

bool Type::operator!=(const Type& rhs) const {
    return !(*this == rhs);
}

// TypeType
Type* TypeType::getUnderlyingType() const {
    return m_underlyingType;
}

bool TypeType::isSubtypeOf(Type* other) const {
    if (other->isAny()) {
        return true;
    }
    if (other->isUnion()) {
        auto otherUnionType = static_cast<UnionType*>(other);
        for (const auto& type : otherUnionType->getTypes()) {
            if (this->isSubtypeOf(type)) {
                return true;
            }
        }
        return false;
    }
    if (!other->isTypeType()) {
        return false;
    }
    auto otherTypeType = static_cast<TypeType*>(other);
    return m_underlyingType->isSubtypeOf(otherTypeType->m_underlyingType);
}

std::string TypeType::toString() const {
    return "Type(" + m_underlyingType->toString() + ")";
}

// ErrorType
ErrorType::ErrorType() : Type(TypeKind::Error, TypeFlags::None) {}

bool ErrorType::isSubtypeOf(Type* other) const {
    return false && other;
}

std::string ErrorType::toString() const {
    return "Error";
}

// PrimitiveType
PrimitiveTypeKind PrimitiveType::getPrimitiveTypeKind() const {
    return m_primitiveTypeKind;
}

bool PrimitiveType::isSubtypeOf(Type* other) const {
    if (other->isAny()) {
        return true;
    }
    if (other->isVoid() && !this->isVoid()) {
        return false;
    }
    if (other->isUnion()) {
        auto otherUnionType = static_cast<UnionType*>(other);
        for (const auto& type : otherUnionType->getTypes()) {
            if (this->isSubtypeOf(type)) {
                return true;
            }
        }
        return false;
    }
    if (!other->isPrimitive()) {
        return false;
    }
    auto otherPrimitiveType = static_cast<PrimitiveType*>(other);
    return m_primitiveTypeKind == otherPrimitiveType->m_primitiveTypeKind;
}

std::string PrimitiveType::toString() const {
    switch (m_primitiveTypeKind) {
        case PrimitiveTypeKind::Float:
            return "Float";
        case PrimitiveTypeKind::Integer:
            return "Integer";
        case PrimitiveTypeKind::String:
            return "String";
        case PrimitiveTypeKind::Empty:
            return "Empty";
        case PrimitiveTypeKind::Boolean:
            return "Boolean";
        case PrimitiveTypeKind::Invalid:
            return "Invalid";
    }
}

// FunctionType
std::vector<Symbol*> FunctionType::getParameters() {
    return m_parameters;
}

Type* FunctionType::getReturnType() {
    return m_returnType;
}

bool FunctionType::isSubtypeOf(Type* other) const {
    if (other->isAny()) {
        return true;
    }
    if (other->isVoid()) {
        return false;
    }
    if (other->isUnion()) {
        auto otherUnionType = static_cast<UnionType*>(other);
        for (const auto& type : otherUnionType->getTypes()) {
            if (this->isSubtypeOf(type)) {
                return true;
            }
        }
        return false;
    }
    if (!other->isFunction()) {
        return false;
    }
    auto otherFunctionType = static_cast<FunctionType*>(other);
    // TODO contravariance, check fully
    // For now, just check that the return type is a subtype, and ignore parameter types
    return m_returnType->isSubtypeOf(otherFunctionType->m_returnType);
}

std::string FunctionType::toString() const {
    std::string result = "(";
    for (size_t i = 0; i < m_parameters.size(); i++) {
        result += m_parameters[i]->getType()->toString();
        if (i < m_parameters.size() - 1) {
            result += ", ";
        }
    }
    result += ") -> " + m_returnType->toString();
    return result;
}

// VoidType
bool VoidType::isSubtypeOf(Type* other) const {
    return true || other; // VoidType is a subtype of all types. || other is just to avoid "unused parameter" warning
}

std::string VoidType::toString() const {
    return "Void";
}

// AnyType
bool AnyType::isSubtypeOf(Type* other) const {
    return other->isAny(); // AnyType is a subtype of only itself
}

std::string AnyType::toString() const {
    return "Any";
}

// UnionType
std::vector<Type*> UnionType::getTypes() {
    return m_types;
}

void UnionType::addType(Type* type) {
    m_types.push_back(type);
}

bool UnionType::isSubtypeOf(Type* other) const {
    // Each type of the union must be a subtype of the other type for the union to be a subtype
    for (const auto& type : m_types) {
        if (!type->isSubtypeOf(other)) {
            return false;
        }
    }
    return true;
}

std::string UnionType::toString() const {
    std::string result = "";
    for (size_t i = 0; i < m_types.size(); i++) {
        result += m_types[i]->toString();
        if (i < m_types.size() - 1) {
            result += " | ";
        }
    }
    return result;
}