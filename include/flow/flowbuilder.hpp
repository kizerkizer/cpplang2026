#pragma once

#include <memory>
#include <stack>

#include "binder/scope.hpp"
#include "flow/flowgraph.hpp"
#include "parser/node.hpp"
#include "parser/nodemap.hpp"
#include "diagnostics/diagnostics.hpp"

struct LoopContext {
    FlowNode* header;
    FlowNode* successor;
};

struct FunctionContext {
    FlowNode* successor;
};

struct FlowContext {
    std::stack<LoopContext> loopContexts; // stack
    FunctionContext functionContext; // TODO no nested functions for now FIXME
};

struct FlowInfo {
    FlowNode* flowNode;
    FlowGraph* flowGraph;
    bool reachable;
};

class FlowAnalyzerResult {
private:
    std::vector<std::unique_ptr<FlowGraph>> m_graphs;
    std::unique_ptr<NodeMap<FlowInfo>> m_flowInfoMap;
public:
    std::vector<FlowGraph*> getGraphs();
    void addGraph(std::unique_ptr<FlowGraph> graph);
    NodeMap<FlowInfo>* getFlowInfoMap() const;
    void setFlowInfoMap(std::unique_ptr<NodeMap<FlowInfo>> flowInfoMap);
    FlowNode* getFlowNode(Node* node) const;
    FlowGraph* getFlowGraph(Node* node) const;
    bool isReachable(Node* node) const;
};

class FlowBuilder {
private:
    std::unique_ptr<NodeMap<FlowInfo>> m_flowInfoMap;
    Diagnostics& m_diagnostics;
    FlowGraph* buildGraphInternal(Node* node, FlowAnalyzerResult* result_out);
    FlowNode* buildFlowNode(FlowGraph* graph, Node* node, FlowNode* successor, FlowContext& context, FlowAnalyzerResult* result_out);
    FlowNode* getFlowNode(Node* node) const;
    void setFlowNode(Node* node, FlowNode* flowNode);
    FlowGraph* getFlowGraph(Node* node) const;
    void setFlowGraph(Node* node, FlowGraph* flowGraph);
public:
    FlowBuilder(Diagnostics& diagnostics);
    std::unique_ptr<FlowAnalyzerResult> buildGraph(Node* rootNode);
    NodeMap<FlowInfo>* getFlowInfoMap() const;
    std::unique_ptr<NodeMap<FlowInfo>> takeFlowInfoMap();
};