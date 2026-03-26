#pragma once

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

enum class PrimitiveType {
    Number, // TODO eventually this is implemented as a union Integer | Float
    String,
    Float,
    Integer,
    Boolean,
    Empty,
};

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