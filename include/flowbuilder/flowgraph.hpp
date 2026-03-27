#pragma once

#include <memory>
#include <vector>

#include "flower/flownode.hpp"

class FlowGraph {
private:
    std::vector<std::unique_ptr<FlowNode>> nodes;
    std::unique_ptr<FlowNode> entry;
    std::unique_ptr<FlowNode> exit;
public:
    FlowGraph();
    FlowNode* getEntry();
    FlowNode* getExit();
    void addNode(std::unique_ptr<FlowNode> node);
    void addEdge(FlowNode* from, FlowNode* to);
    bool isReachable(FlowNode* start, FlowNode* to);
    std::vector<FlowNode*> getUnreachable();
};
