#pragma once

#include <map>

#include "binder/symbol.hpp"

using SymbolTypes = struct SymbolTypes {
    std::map<Symbol, Type> narrowedTypes;
};

class FlowNodeSymbolTypesMap {
private:
    std::map<int, SymbolTypes> m_map;
public:
    FlowNodeSymbolTypesMap() = default;
    void updateSymbolType(FlowNode* flowNode, const Symbol& symbol, const Type& type);
    void cloneSymbolTypes(FlowNode* from, FlowNode* to);
    SymbolTypes* getSymbolTypes(FlowNode* flowNode);
    Type* getSymbolType(FlowNode* flowNode, const Symbol& symbol);
};