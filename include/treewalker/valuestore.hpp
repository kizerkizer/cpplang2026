#pragma once

#include <vector>

#include "checker/type.hpp"
#include "checker/typestore.hpp"
#include "treewalker/value.hpp"
#include "parser/node.hpp"

class ValueStore {
private:
    TypeStore* m_typeStore;
    std::vector<std::unique_ptr<Value>> m_values;
public:
    ValueStore(TypeStore* typeStore) : m_typeStore(typeStore) {};
    void addValue(std::unique_ptr<Value> value);
    IntegerValue* makeIntegerValue(long long value);
    FloatValue* makeFloatValue(double value);
    StringValue* makeStringValue(const std::string& value);
    BooleanValue* makeBooleanValue(bool value);
    EmptyValue* makeEmptyValue();
    VoidValue* makeVoidValue();
    FunctionValue* makeFunctionValue(FunctionDeclarationNode* node, FunctionType* functionType, Environment* definingEnvironment);
    ReturnValue* makeReturnValue(Value* value);
    BreakValue* makeBreakValue();
    ContinueValue* makeContinueValue();
    TypeValue* makeTypeValue(Type* type);
};