#include <print>

#include "treewalker/environment.hpp"

void Environment::setVar(const std::string& name, Value* value) {
    if (this->vars.contains(name)) {
        this->vars[name] = value;
    } else if (this->parent) {
        this->parent->setVar(name, value);
    } else {
        std::print("Error: Cannot set variable '{}'; not defined\n", name);
    }
}

void Environment::defineVar(const std::string& name, Value* value) {
    this->vars[name] = value;
}

Value* Environment::getVar(const std::string& name) const {
    auto it = this->vars.find(name);
    if (it != this->vars.end()) {
        return it->second;
    } else if (this->parent) {
        return this->parent->getVar(name);
    } else {
        return nullptr;
    }
}

Environment* Environment::getParent() const {
    return this->parent;
}

void Environment::addChildEnvironment(std::unique_ptr<Environment> childEnvironment) {
    this->childEnvironments.push_back(std::move(childEnvironment));
}

std::vector<Environment*> Environment::getChildEnvironments() const {
    std::vector<Environment*> result;
    for (const auto& child : this->childEnvironments) {
        result.push_back(child.get());
    }
    return result;
}

std::vector<std::unique_ptr<Environment>> Environment::takeChildEnvironments() {
    return std::move(this->childEnvironments);
}

//ValueStore
void ValueStore::addValue(std::unique_ptr<Value> value) {
    this->values.push_back(std::move(value));
}

ReturnValue* ValueStore::makeReturnValue(Value* value) {
    auto returnValue = std::make_unique<ReturnValue>(value);
    auto returnValuePtr = returnValue.get();
    this->addValue(std::move(returnValue));
    return returnValuePtr;
}

BreakValue* ValueStore::makeBreakValue() {
    auto breakValue = std::make_unique<BreakValue>();
    auto breakValuePtr = breakValue.get();
    this->addValue(std::move(breakValue));
    return breakValuePtr;
}

ContinueValue* ValueStore::makeContinueValue() {
    auto continueValue = std::make_unique<ContinueValue>();
    auto continueValuePtr = continueValue.get();
    this->addValue(std::move(continueValue));
    return continueValuePtr;
}

FunctionValue* ValueStore::makeFunctionValue(Node* functionDeclarationNode, Environment* definingEnvironment) {
    auto functionValue = std::make_unique<FunctionValue>(functionDeclarationNode, definingEnvironment);
    auto functionValuePtr = functionValue.get();
    this->addValue(std::move(functionValue));
    return functionValuePtr;
}

IntegerValue* ValueStore::makeIntegerValue(int value) {
    auto integerValue = std::make_unique<IntegerValue>(value);
    auto integerValuePtr = integerValue.get();
    this->addValue(std::move(integerValue));
    return integerValuePtr;
}

FloatValue* ValueStore::makeFloatValue(double value) {
    auto floatValue = std::make_unique<FloatValue>(value);
    auto floatValuePtr = floatValue.get();
    this->addValue(std::move(floatValue));
    return floatValuePtr;
}

StringValue* ValueStore::makeStringValue(const std::string& value) {
    auto stringValue = std::make_unique<StringValue>(value);
    auto stringValuePtr = stringValue.get();
    this->addValue(std::move(stringValue));
    return stringValuePtr;
}

BooleanValue* ValueStore::makeBooleanValue(bool value) {
    auto booleanValue = std::make_unique<BooleanValue>(value);
    auto booleanValuePtr = booleanValue.get();
    this->addValue(std::move(booleanValue));
    return booleanValuePtr;
}

EmptyValue* ValueStore::makeEmptyValue() {
    auto emptyValue = std::make_unique<EmptyValue>();
    auto emptyValuePtr = emptyValue.get();
    this->addValue(std::move(emptyValue));
    return emptyValuePtr;
}

VoidValue* ValueStore::makeVoidValue() {
    auto voidValue = std::make_unique<VoidValue>();
    auto voidValuePtr = voidValue.get();
    this->addValue(std::move(voidValue));
    return voidValuePtr;
}