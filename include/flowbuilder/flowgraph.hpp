#pragma once

#include <memory>
#include <vector>

#include "flowbuilder/flownode.hpp"
#include "parser/nodemap.hpp"

using FlowGraphInfo = struct FlowGraphInfo {
    bool reachable;
};

class FlowGraph {
private:
    std::unique_ptr<NodeMap<FlowGraphInfo>> m_flowGraphNodeMap;
    std::vector<std::unique_ptr<FlowNode>> m_nodes;
    std::unique_ptr<FlowNode> m_entry = nullptr;
    std::unique_ptr<FlowNode> m_exit = nullptr;
    Node* m_astNode = nullptr;
    void setReachable(Node* node, bool reachable);
    bool getReachable(Node* node);
public:
    FlowGraph();
    FlowNode* getEntry();
    FlowNode* getExit();
    std::vector<FlowNode*> getNodes();
    FlowNode* addNode(std::unique_ptr<FlowNode> node);
    void addEdge(FlowNode* from, FlowNode* to, std::optional<std::string> edgeName = std::nullopt);
    bool isReachable(FlowNode* start, FlowNode* to);
    bool isASTNodeReachable(Node* node);
    std::vector<FlowNode*> getUnreachable();
    Node* getAstNode();
    void setAstNode(Node* node);
    void assignReachabilityToNodes();
    NodeMap<FlowGraphInfo>* getFlowGraphNodeMap() const;
    std::unique_ptr<NodeMap<FlowGraphInfo>> takeFlowGraphNodeMap();
};
