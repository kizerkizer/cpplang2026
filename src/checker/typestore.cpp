#include "checker/typestore.hpp"

// TypeStore
Type* TypeStore::simplifyUnionType(UnionType* unionType) {
    std::vector<Type*> uniqueTypes;
    for (auto& type : unionType->getTypes()) {
        auto it = std::find_if(uniqueTypes.begin(), uniqueTypes.end(), [&type](const Type* uniqueType) { return *type == *uniqueType; });
        if (it != uniqueTypes.end()) {
            continue;
        }
        if (type->getTypeKind() == TypeKind::Union) {
            auto resultType = this->simplifyUnionType(static_cast<UnionType*>(type));
            if (resultType->getTypeKind() == TypeKind::Union) {
                auto resultUnionType = static_cast<UnionType*>(resultType);
                for (auto& resultUnionTypeMember : resultUnionType->getTypes()) {
                    auto it = std::find_if(uniqueTypes.begin(), uniqueTypes.end(), [&resultUnionTypeMember](const Type* uniqueType) { return *resultUnionTypeMember == *uniqueType; });
                    if (it != uniqueTypes.end()) {
                        continue;
                    }
                    uniqueTypes.push_back(resultUnionTypeMember);
                }
                continue;
            } else {
                uniqueTypes.push_back(resultType);
                continue;
            }
        }
        uniqueTypes.push_back(type);
    }
    if (uniqueTypes.size() == 1) {
        return uniqueTypes.front();
    }
    for (auto& type : uniqueTypes) {
        if (type->getTypeKind() == TypeKind::Any) {
            return this->getAnyType();
        }
    }
    std::unique_ptr<UnionType> simplifiedUnionType = std::make_unique<UnionType>();
    for (auto& type : uniqueTypes) {
        simplifiedUnionType->addType(type);
    }
    auto simplifiedUnionTypePointer = simplifiedUnionType.get();
    this->m_types.push_back(std::move(simplifiedUnionType));
    return simplifiedUnionTypePointer;
}

Type* TypeStore::simplifyType(Type* type) {
    if (type->getTypeKind() == TypeKind::Union) {
        auto unionType = static_cast<UnionType*>(type);
        return this->simplifyUnionType(unionType);
    }
    return type;
}