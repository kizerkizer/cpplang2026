#include "checker/type.hpp"
#include "binder/symbol.hpp"
#include "parser/node.hpp"

//Type
TypeKind Type::getTypeKind() const {
    return this->kind;
}

TypeFlags Type::getTypeFlags() const {
    return this->flags;
}

void Type::setTypeFlags(TypeFlags flags) {
    this->flags = flags;
}

void Type::setTypeFlag(TypeFlags flag) {
    this->flags = this->flags | flag;
}

bool Type::operator==(const Type& rhs) const {
    return this->isSubtypeOf(const_cast<Type*>(&rhs)) && const_cast<Type*>(&rhs)->isSubtypeOf(const_cast<Type*>(this));
}

bool Type::operator!=(const Type& rhs) const {
    return !(*this == rhs);
}

//PrimitiveType
PrimitiveTypeKind PrimitiveType::getPrimitiveTypeKind() const {
    return this->primitiveTypeKind;
}

bool PrimitiveType::isSubtypeOf(Type* other) const {
    if (other->getTypeKind() == TypeKind::Any) {
        return true;
    }
    if (other->getTypeKind() == TypeKind::Void) {
        return false;
    }
    if (other->getTypeKind() == TypeKind::Union) {
        auto otherUnionType = static_cast<UnionType*>(other);
        for (const auto& type : otherUnionType->getTypes()) {
            if (this->isSubtypeOf(type)) {
                return true;
            }
        }
        return false;
    }
    if (other->getTypeKind() != TypeKind::Primitive) {
        return false;
    }
    auto otherPrimitiveType = static_cast<PrimitiveType*>(other);
    return this->primitiveTypeKind == otherPrimitiveType->primitiveTypeKind;
}

//FunctionType
std::vector<Symbol*> FunctionType::getParameters() {
    return this->parameters;
}

Type* FunctionType::getReturnType() {
    return this->returnType;
}

bool FunctionType::isSubtypeOf(Type* other) const {
    if (other->getTypeKind() == TypeKind::Any) {
        return true;
    }
    if (other->getTypeKind() == TypeKind::Void) {
        return false;
    }
    if (other->getTypeKind() == TypeKind::Union) {
        auto otherUnionType = static_cast<UnionType*>(other);
        for (const auto& type : otherUnionType->getTypes()) {
            if (this->isSubtypeOf(type)) {
                return true;
            }
        }
        return false;
    }
    if (other->getTypeKind() != TypeKind::Function) {
        return false;
    }
    auto otherFunctionType = static_cast<FunctionType*>(other);
    // TODO contravariance, check fully
    // For now, just check that the return type is a subtype, and ignore parameter types
    return this->returnType->isSubtypeOf(otherFunctionType->returnType);
}

//VoidType
bool VoidType::isSubtypeOf(Type* other) const {
    return true || other; // VoidType is a subtype of all types. || other is just to avoid "unused parameter" warning
}

//AnyType
bool AnyType::isSubtypeOf(Type* other) const {
    return other->getTypeKind() == TypeKind::Any; // AnyType is a subtype of only itself
}

//UnionType
std::vector<Type*> UnionType::getTypes() {
    return this->types;
}

void UnionType::addType(Type* type) {
    this->types.push_back(type);
}

bool UnionType::isSubtypeOf(Type* other) const {
    for (const auto& type : this->types) {
        if (!type->isSubtypeOf(other)) {
            return false;
        }
    }
    return true;
}