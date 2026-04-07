#pragma once

#include <memory>
#include <vector>

#include "flowbuilder/flownode.hpp"

class FlowGraph {
private:
    std::vector<std::unique_ptr<FlowNode>> m_nodes;
    std::unique_ptr<FlowNode> m_entry = nullptr;
    std::unique_ptr<FlowNode> m_exit = nullptr;
    Node* m_astNode = nullptr;
public:
    FlowGraph();
    FlowNode* getEntry();
    FlowNode* getExit();
    std::vector<FlowNode*> getNodes();
    FlowNode* addNode(std::unique_ptr<FlowNode> node);
    void addEdge(FlowNode* from, FlowNode* to);
    bool isReachable(FlowNode* start, FlowNode* to);
    std::vector<FlowNode*> getUnreachable();
    Node* getAstNode();
    void setAstNode(Node* node);
    void assignReachabilityToNodes();
};
