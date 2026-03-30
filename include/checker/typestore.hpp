#pragma once

#include <concepts>
#include <vector>
#include <memory>

#include "checker/type.hpp"

class TypeStore {
private:
    std::vector<std::unique_ptr<Type>> types;
    std::unique_ptr<PrimitiveType> stringType = std::make_unique<PrimitiveType>(PrimitiveTypeKind::String);
    std::unique_ptr<PrimitiveType> booleanType = std::make_unique<PrimitiveType>(PrimitiveTypeKind::Boolean);
    std::unique_ptr<PrimitiveType> emptyType = std::make_unique<PrimitiveType>(PrimitiveTypeKind::Empty);
    std::unique_ptr<PrimitiveType> floatType = std::make_unique<PrimitiveType>(PrimitiveTypeKind::Float);
    std::unique_ptr<PrimitiveType> integerType = std::make_unique<PrimitiveType>(PrimitiveTypeKind::Integer);
    std::unique_ptr<AnyType> anyType = std::make_unique<AnyType>();
    std::unique_ptr<VoidType> voidType = std::make_unique<VoidType>();
    Type* simplifyUnionType(UnionType* unionType);
public:
    TypeStore() = default;
    template<std::derived_from<Type> T, typename... Args>
    T* createType(Args&&... args) {
        if constexpr (std::is_same_v<T, PrimitiveType>) {
            std::unique_ptr<PrimitiveType> primitiveType = std::make_unique<PrimitiveType>(std::forward<Args>(args)...);
            PrimitiveTypeKind primitiveTypeKind = primitiveType->getPrimitiveTypeKind();
            switch (primitiveTypeKind) {
                case PrimitiveTypeKind::String:
                    return this->getStringType();
                case PrimitiveTypeKind::Boolean:
                    return this->getBooleanType();
                case PrimitiveTypeKind::Empty:
                    return this->getEmptyType();
                case PrimitiveTypeKind::Float:
                    return this->getFloatType();
                case PrimitiveTypeKind::Integer:
                    return this->getIntegerType();
                case PrimitiveTypeKind::Invalid:
                    return nullptr;
            }
        }
        auto type = std::make_unique<T>(std::forward<Args>(args)...);
        T* typePointer = type.get();
        this->types.push_back(std::move(type));
        return typePointer;
    }
    Type* simplifyType(Type* type);
    PrimitiveType* getStringType() const {
        return this->stringType.get();
    }
    PrimitiveType* getBooleanType() const {
        return this->booleanType.get();
    }
    PrimitiveType* getEmptyType() const {
        return this->emptyType.get();
    }
    PrimitiveType* getFloatType() const {
        return this->floatType.get();
    }
    PrimitiveType* getIntegerType() const {
        return this->integerType.get();
    }
    AnyType* getAnyType() const {
        return this->anyType.get();
    }
    VoidType* getVoidType() const {
        return this->voidType.get();
    }
};