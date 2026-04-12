#pragma once

#include <string>
#include <vector>

#include "common/programuniqueid.hpp"

class Symbol; // defined in binder/symbol.hpp

enum class TypeKind {
    Error,
    TypeType, // The type of types
    Primitive,
    Void, // Bottom type, has no values, is a subtype of all types, supertype of no types
    Any, // Top type, has any value, is a supertype of all types, subtype of no types
    // TODO
    // !!!! Should Void, Any be primitive types?
    Class, // TODO
    Extensional, // TODO
    ExtensionalMember, // TODO
    Intensional, // TODO
    Function,
    Union,
};

enum class PrimitiveTypeKind {
    String,
    Float,
    Integer,
    Boolean,
    Empty,
    Invalid,
};

constexpr const char* typeKindToString(TypeKind typeKind) {
    switch (typeKind) {
        case TypeKind::Error:
            return "Error";
        case TypeKind::TypeType:
            return "TypeType";
        case TypeKind::Primitive:
            return "Primitive";
        case TypeKind::Void:
            return "Void";
        case TypeKind::Any:
            return "Any";
        case TypeKind::Class:
            return "Class";
        case TypeKind::Extensional:
            return "Extensional";
        case TypeKind::ExtensionalMember:
            return "ExtensionalMember";
        case TypeKind::Intensional:
            return "Intensional";
        case TypeKind::Function:
            return "Function";
        case TypeKind::Union:
            return "Union";
    }
}

constexpr const char* primitiveTypeToString(PrimitiveTypeKind primitiveType) {
    switch (primitiveType) {
        case PrimitiveTypeKind::String:
            return "String";
        case PrimitiveTypeKind::Boolean:
            return "Boolean";
        case PrimitiveTypeKind::Empty:
            return "Empty";
        case PrimitiveTypeKind::Float:
            return "Float";
        case PrimitiveTypeKind::Integer:
            return "Integer";
        case PrimitiveTypeKind::Invalid:
            return "Invalid";
    }
}

constexpr bool isPrimitiveTypeNumeric(PrimitiveTypeKind primitiveType) {
    return primitiveType == PrimitiveTypeKind::Integer || primitiveType == PrimitiveTypeKind::Float;
}

enum class TypeFlags {
    None = 0,
    Inferred = 1 << 0,
    HasTypeParameters = 1 << 1,
    Primitive = 1 << 2,
    UserDefined = 1 << 3,
};

inline TypeFlags operator|(TypeFlags a, TypeFlags b) {
    return static_cast<TypeFlags>(static_cast<int>(a) | static_cast<int>(b));
}

class Type {
private:
    int m_id;
    TypeKind m_kind;
    TypeFlags m_flags;
public:
    Type(TypeKind kind, TypeFlags flags) : m_id(getNextId()), m_kind(kind), m_flags(flags) {};
    virtual ~Type() = default;
    TypeKind getTypeKind() const;
    TypeFlags getTypeFlags() const;
    void setTypeFlags(TypeFlags flags);
    void setTypeFlag(TypeFlags flag);
    virtual bool isSubtypeOf(Type* other) const = 0;
    virtual std::string toString() const = 0;
    bool isPrimitive() const;
    bool isUnion() const;
    bool isVoid() const;
    bool isAny() const;
    bool isFunction() const;
    bool isError() const;
    bool isBoolean() const;
    bool isInteger() const;
    bool isFloat() const;
    bool isString() const;
    bool isEmpty() const;
    bool isTypeType() const;
    int getId() const;
    bool operator==(const Type& rhs) const; // TODO needed?
    bool operator!=(const Type& rhs) const; // TODO needed?
};

// Represents the type of a value that is a type. The type of a type.
class TypeType : public Type {
private:
    Type* m_underlyingType;
public:
    TypeType(Type* underlyingType) : Type(TypeKind::TypeType, TypeFlags::None), m_underlyingType(underlyingType) {};
    bool isSubtypeOf(Type* other) const override;
    std::string toString() const override;
    Type* getUnderlyingType() const;
};

// Used when error encountered in type checking. Not a valid language type.
class ErrorType : public Type {
public:
    ErrorType();
    bool isSubtypeOf(Type* other) const override;
    std::string toString() const override;
};

class VoidType : public Type {
public:
    VoidType() : Type(TypeKind::Void, TypeFlags::None) {};
    bool isSubtypeOf(Type* other) const override;
    std::string toString() const override;
};

class AnyType : public Type {
public:
    AnyType() : Type(TypeKind::Any, TypeFlags::None) {};
    bool isSubtypeOf(Type* other) const override;
    std::string toString() const override;
};

class PrimitiveType : public Type {
private:
    PrimitiveTypeKind m_primitiveTypeKind;
public:
    PrimitiveType(PrimitiveTypeKind primitiveTypeKind) : Type(TypeKind::Primitive, TypeFlags::Primitive), m_primitiveTypeKind(primitiveTypeKind) {};
    PrimitiveTypeKind getPrimitiveTypeKind() const;
    bool isSubtypeOf(Type* other) const override;
    std::string toString() const override;
};

class FunctionType : public Type {
private:
    std::vector<Symbol*> m_parameters;
    Type* m_returnType;
public:
    FunctionType(std::vector<Symbol*> parameters, Type* returnType) : Type(TypeKind::Function, TypeFlags::None), m_parameters(parameters), m_returnType(std::move(returnType)) {};
    std::vector<Symbol*> getParameters();
    Type* getReturnType();
    bool isSubtypeOf(Type* other) const override;
    std::string toString() const override;
};

class UnionType : public Type {
private:
    std::vector<Type*> m_types;
public:
    template<typename... Args>
    UnionType(Args&& ...types) : Type(TypeKind::Union, TypeFlags::None) {
        this->m_types.reserve(sizeof...(types));
        (this->m_types.push_back(std::forward<Args>(types)), ...);
    };
    std::vector<Type*> getTypes();
    void addType(Type* type);
    bool isSubtypeOf(Type* other) const override;
    std::string toString() const override;
};