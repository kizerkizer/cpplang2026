#include "checker/flownodesymboltypesmap.hpp"

void FlowNodeSymbolTypesMap::updateSymbolType(FlowNode* flowNode, const Symbol& symbol, const Type& type) {
    m_map[flowNode->getId()].narrowedTypes[symbol] = type;
}

void FlowNodeSymbolTypesMap::cloneSymbolTypes(FlowNode* from, FlowNode* to) {
    m_map[to->getId()] = m_map[from->getId()];
}

SymbolTypes* FlowNodeSymbolTypesMap::getSymbolTypes(FlowNode* flowNode) {
    auto it = m_map.find(flowNode->getId());
    if (it != m_map.end()) {
        return &it->second;
    }
    return nullptr;
}

Type* FlowNodeSymbolTypesMap::getSymbolType(FlowNode* flowNode, const Symbol& symbol) {
    auto it = m_map.find(flowNode->getId());
    if (it != m_map.end()) {
        auto& symbolTypes = it->second.narrowedTypes;
        if (symbolTypes.contains(symbol)) {
            return &symbolTypes[symbol];
        }
    }
    return nullptr;
}