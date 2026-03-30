#pragma once

#include <string>
#include <memory>
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

#define IS_PRIMITIVE_TYPE_NUMERIC(primitiveTypeKind) (primitiveTypeKind == PrimitiveTypeKind::Float || primitiveTypeKind == PrimitiveTypeKind::Integer)

std::string primitiveTypeToString (PrimitiveTypeKind primitiveType);

std::string typeKindToString(TypeKind typeKind);

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
public:
    Type(TypeKind kind, TypeFlags flags) : kind(kind), flags(flags) {};
    virtual ~Type() = default;
    TypeKind getTypeKind() const;
    TypeFlags getTypeFlags() const;
    void setTypeFlags(TypeFlags flags);
    void setTypeFlag(TypeFlags flag);
    virtual bool isSubtypeOf(Type* other) const = 0;
    bool operator==(const Type& rhs) const;
    bool operator!=(const Type& rhs) const;
private:
    TypeKind kind;
    TypeFlags flags;
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
public:
    PrimitiveType(PrimitiveTypeKind primitiveTypeKind) : Type(TypeKind::Primitive, TypeFlags::Primitive), primitiveTypeKind(primitiveTypeKind) {};
    PrimitiveTypeKind getPrimitiveTypeKind() const;
    bool isSubtypeOf(Type* other) const override;
    /*bool operator==(const Type& rhs) const override;
    bool operator!=(const Type& rhs) const override;*/
private:
    PrimitiveTypeKind primitiveTypeKind;
};

class FunctionType : public Type {
public:
    FunctionType(std::vector<Symbol*> parameters, Type* returnType) : Type(TypeKind::Function, TypeFlags::None), parameters(parameters), returnType(std::move(returnType)) {};
    std::vector<Symbol*> getParameters();
    //std::vector<Type*> getParameterTypes();
    Type* getReturnType();
    bool isSubtypeOf(Type* other) const override;
    /*bool operator==(const Type& rhs) const override;
    bool operator!=(const Type& rhs) const override;*/
private:
    //std::vector<Type*> parameterTypes;
    std::vector<Symbol*> parameters;
    Type* returnType;
};

class UnionType : public Type {
public:
    template<typename... Args>
    UnionType(Args&& ...types) : Type(TypeKind::Union, TypeFlags::None) {
        this->types.reserve(sizeof...(types));
        (this->types.push_back(std::forward<Args>(types)), ...);
    };
    std::vector<Type*> getTypes();
    void addType(Type* type);
    bool isSubtypeOf(Type* other) const override;
    std::unique_ptr<Type> simplify(); // TODO
private:
    std::vector<Type*> types;
};