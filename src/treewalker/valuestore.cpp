#include "treewalker/valuestore.hpp"
#include "checker/type.hpp"
#include "parser/node.hpp"

//ValueStore
void ValueStore::addValue(std::unique_ptr<Value> value) {
    this->m_values.push_back(std::move(value));
}

ReturnValue* ValueStore::makeReturnValue(Value* value) {
    auto returnValue = std::make_unique<ReturnValue>(value, m_typeStore->getVoidType());
    auto returnValuePtr = returnValue.get();
    this->addValue(std::move(returnValue));
    return returnValuePtr;
}

BreakValue* ValueStore::makeBreakValue() {
    auto breakValue = std::make_unique<BreakValue>(m_typeStore->getVoidType());
    auto breakValuePtr = breakValue.get();
    this->addValue(std::move(breakValue));
    return breakValuePtr;
}

ContinueValue* ValueStore::makeContinueValue() {
    auto continueValue = std::make_unique<ContinueValue>(m_typeStore->getVoidType());
    auto continueValuePtr = continueValue.get();
    this->addValue(std::move(continueValue));
    return continueValuePtr;
}

FunctionValue* ValueStore::makeFunctionValue(FunctionDeclarationNode* node, FunctionType* functionType, Environment* definingEnvironment) {
    auto functionValue = std::make_unique<FunctionValue>(definingEnvironment, node, functionType);
    auto functionValuePtr = functionValue.get();
    this->addValue(std::move(functionValue));
    return functionValuePtr;
}

IntegerValue* ValueStore::makeIntegerValue(long long value) {
    auto integerValue = std::make_unique<IntegerValue>(value, m_typeStore->getIntegerType());
    auto integerValuePtr = integerValue.get();
    this->addValue(std::move(integerValue));
    return integerValuePtr;
}

FloatValue* ValueStore::makeFloatValue(double value) {
    auto floatValue = std::make_unique<FloatValue>(value, m_typeStore->getFloatType());
    auto floatValuePtr = floatValue.get();
    this->addValue(std::move(floatValue));
    return floatValuePtr;
}

StringValue* ValueStore::makeStringValue(const std::string& value) {
    auto stringValue = std::make_unique<StringValue>(value, m_typeStore->getStringType());
    auto stringValuePtr = stringValue.get();
    this->addValue(std::move(stringValue));
    return stringValuePtr;
}

BooleanValue* ValueStore::makeBooleanValue(bool value) {
    auto booleanValue = std::make_unique<BooleanValue>(value, m_typeStore->getBooleanType());
    auto booleanValuePtr = booleanValue.get();
    this->addValue(std::move(booleanValue));
    return booleanValuePtr;
}

EmptyValue* ValueStore::makeEmptyValue() {
    auto emptyValue = std::make_unique<EmptyValue>(m_typeStore->getEmptyType());
    auto emptyValuePtr = emptyValue.get();
    this->addValue(std::move(emptyValue));
    return emptyValuePtr;
}

VoidValue* ValueStore::makeVoidValue() {
    auto voidValue = std::make_unique<VoidValue>(m_typeStore->getVoidType());
    auto voidValuePtr = voidValue.get();
    this->addValue(std::move(voidValue));
    return voidValuePtr;
}

TypeValue* ValueStore::makeTypeValue(Type* type) {
    auto typeValue = std::make_unique<TypeValue>(type);
    auto typeValuePtr = typeValue.get();
    this->addValue(std::move(typeValue));
    return typeValuePtr;
}