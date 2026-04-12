#pragma once

#include <memory>
#include <unordered_map>

#include "parser/node.hpp"

/**
 * @brief A map that associates nodes with values of type T. 
 * Use to add additional information to nodes without modifying the node
 * classes themselves. For example, can be used to store type information
 * for nodes after type checking, or to store flow graph info for nodes
 * after flow graph construction. The NodeMap owns the pointer values via
 * std::unique_ptr, so it is responsible for managing their lifetimes.
 * 
 * @tparam T The type of values to be stored in the map.
 */
template<typename T>
class NodeMap {
private:
    std::unordered_map<int, std::unique_ptr<T>> m_map;
public:
    void setValue(Node* node, std::unique_ptr<T> value) {
        m_map[node->getId()] = std::move(value);
    }
    T* getValue(Node* node) const {
        auto it = m_map.find(node->getId());
        if (it != m_map.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    std::unique_ptr<T> takeValue(Node* node) {
        auto it = m_map.find(node->getId());
        if (it != m_map.end()) {
            std::unique_ptr<T> value = std::move(it->second);
            m_map.erase(it);
            return value;
        }
        return nullptr;
    }
};