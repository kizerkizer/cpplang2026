#include "checker/type.hpp"
#include "binder/name.hpp"

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
        case PrimitiveTypeKind::Number:
            return "Number";
        case PrimitiveTypeKind::Invalid:
            return "Invalid";
    }
}

FunctionType::FunctionType(std::vector<Name*> parameters, std::unique_ptr<Type> returnType) {
    this->parameters = parameters;
    this->returnType = std::move(returnType);
}

std::vector<Name*> FunctionType::getParameters() {
    return this->parameters;
}

Type* FunctionType::getReturnType() {
    return this->returnType.get();
}