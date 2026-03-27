#pragma once

#include <string>
#include <memory>
#include <vector>

class Name; // defined in binder/name.hpp

enum class TypeKind {
    Primitive,
    Class,
    Extensional,
    ExtensionalMember,
    Intensional,
    Function,
    Top,
    Bottom,
};

enum class PrimitiveTypeKind {
    Number, // TODO eventually this is implemented as a union Integer | Float
    String,
    Float,
    Integer,
    Boolean,
    Empty,
    Invalid,
};

std::string primitiveTypeToString (PrimitiveTypeKind primitiveType);

enum class TypeFlags {
    None = 0,
    Inferred = 1 << 0,
    HasTypeParameters = 1 << 1,
    Primitive = 1 << 2,
    UserDefined = 1 << 3,
};

class Type {
public:
    TypeKind getTypeKind();
    TypeFlags getTypeFlags();
    void setTypeFlags(TypeFlags flags);
    void setTypeFlag(TypeFlags flag);
    bool isSubtypeOf(Type* other);
    bool isSupertypeOf(Type* other);
private:
    TypeKind kind;
    TypeFlags flags;
};

class PrimitiveType : Type {
public:
    PrimitiveTypeKind getPrimitiveTypeKind();
private:
    PrimitiveTypeKind primitiveTypeKind;
};

class FunctionType : Type {
public:
    FunctionType(std::vector<Name*> parameters, std::unique_ptr<Type> returnType);
    std::vector<Name*> getParameters();
    Type* getReturnType();
private:
    std::vector<Name*> parameters;
    std::unique_ptr<Type> returnType;
};

class UnionType : Type {
public:
    UnionType();
    std::vector<Type*> getTypes();
    void addType(std::unique_ptr<Type> type);
private:
    std::vector<std::unique_ptr<Type>> types;
};