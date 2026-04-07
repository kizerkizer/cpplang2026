#pragma once

#include <vector>

class Symbol; // defined in binder/symbol.hpp

enum class TypeKind {
    Primitive,
    Void, // Bottom type, has no values, is a subtype of all types, supertype of no types
    Any, // Top type, has any value, is a supertype of all types, subtype of no types
    Class, // TODO
    Extensional, // TODO
    ExtensionalMember, // TODO
    Intensional, // TODO
    Function,
    Union,
};

enum class PrimitiveTypeKind {
    //Number, // TODO eventually this is implemented as a union Integer | Float
    String,
    Float,
    Integer,
    Boolean,
    Empty,
    Invalid,
};

constexpr const char* typeKindToString(TypeKind typeKind) {
    switch (typeKind) {
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
    TypeKind m_kind;
    TypeFlags m_flags;
public:
    Type(TypeKind kind, TypeFlags flags) : m_kind(kind), m_flags(flags) {};
    virtual ~Type() = default;
    TypeKind getTypeKind() const;
    TypeFlags getTypeFlags() const;
    void setTypeFlags(TypeFlags flags);
    void setTypeFlag(TypeFlags flag);
    virtual bool isSubtypeOf(Type* other) const = 0;
    bool operator==(const Type& rhs) const;
    bool operator!=(const Type& rhs) const;
};

class VoidType : public Type {
public:
    VoidType() : Type(TypeKind::Void, TypeFlags::None) {};
    bool isSubtypeOf(Type* other) const override;
};

class AnyType : public Type {
public:
    AnyType() : Type(TypeKind::Any, TypeFlags::None) {};
    bool isSubtypeOf(Type* other) const override;
};

class PrimitiveType : public Type {
private:
    PrimitiveTypeKind m_primitiveTypeKind;
public:
    PrimitiveType(PrimitiveTypeKind primitiveTypeKind) : Type(TypeKind::Primitive, TypeFlags::Primitive), m_primitiveTypeKind(primitiveTypeKind) {};
    PrimitiveTypeKind getPrimitiveTypeKind() const;
    bool isSubtypeOf(Type* other) const override;
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
};