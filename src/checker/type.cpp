#include <memory>

#include "checker/type.hpp"
#include "binder/symbol.hpp"
#include "parser/node.hpp"

std::string primitiveTypeToString(PrimitiveTypeKind primitiveType) {
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

std::string typeKindToString(TypeKind typeKind) {
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

/*bool PrimitiveType::operator==(const Type& rhs) const {
    if (rhs.getTypeKind() != TypeKind::Primitive) {
        return false;
    }
    auto rhsPrimitiveType = static_cast<const PrimitiveType*>(&rhs);
    return this->primitiveTypeKind == rhsPrimitiveType->primitiveTypeKind;
}

bool PrimitiveType::operator!=(const Type& rhs) const {
    return !(*this == rhs);
}*/

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

/**
 * @brief Simplifies the union type by removing duplicate types. If the union type only has one unique type, then that type is returned instead of a new UnionType.
 * This destroys the original UnionType and its types, so it should only be called on a UnionType that is not used anywhere else.
 * Maybe later this will not destroy the original UnionType, but for now this is simpler to implement.
 * 
 * @return std::unique_ptr<Type> The simplified type, which could be a single type or a new UnionType.
 */
std::unique_ptr<Type> UnionType::simplify() {
    // TODO
    std::vector<Type*> uniqueTypes;
    for (auto& type : this->types) {
        auto it = std::find_if(uniqueTypes.begin(), uniqueTypes.end(), [&type](const Type* uniqueType) { return *type == *uniqueType; });
        if (it != uniqueTypes.end()) {
            continue;
        }
        uniqueTypes.push_back(type);
    }
    if (uniqueTypes.size() == 1) {
        return std::unique_ptr<Type>(uniqueTypes.front());
    }
    std::unique_ptr<UnionType> simplifiedUnionType = std::make_unique<UnionType>();
    for (auto& type : uniqueTypes) {
        simplifiedUnionType->addType(type);
    }
    return simplifiedUnionType;
}

bool UnionType::isSubtypeOf(Type* other) const {
    for (const auto& type : this->types) {
        if (!type->isSubtypeOf(other)) {
            return false;
        }
    }
    return true;
}