#include "checker/typestore.hpp"

// TypeStore
Type* TypeStore::simplifyUnionType(UnionType* unionType) {
    std::vector<Type*> uniqueTypes;
    for (auto& type : unionType->getTypes()) {
        auto it = std::find_if(uniqueTypes.begin(), uniqueTypes.end(), [&type](const Type* uniqueType) { return *type == *uniqueType; });
        if (it != uniqueTypes.end()) {
            continue;
        }
        uniqueTypes.push_back(type);
    }
    if (uniqueTypes.size() == 1) {
        return uniqueTypes.front();
    }
    std::unique_ptr<UnionType> simplifiedUnionType = std::make_unique<UnionType>();
    for (auto& type : uniqueTypes) {
        simplifiedUnionType->addType(type);
    }
    auto simplifiedUnionTypePointer = simplifiedUnionType.get();
    this->types.push_back(std::move(simplifiedUnionType));
    return simplifiedUnionTypePointer;
}

Type* TypeStore::simplifyType(Type* type) {
    if (type->getTypeKind() == TypeKind::Union) {
        auto unionType = static_cast<UnionType*>(type);
        return this->simplifyUnionType(unionType);
    }
    return type;
}