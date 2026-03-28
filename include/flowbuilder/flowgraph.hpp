#pragma once

#include <memory>
#include <vector>

#include "flowbuilder/flownode.hpp"

class FlowGraph {
private:
    std::vector<std::unique_ptr<FlowNode>> nodes;
    std::unique_ptr<FlowNode> entry = nullptr;
    std::unique_ptr<FlowNode> exit = nullptr;
    Node* astNode = nullptr;
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
};
