#pragma once

#include <memory>
#include <stack>

#include "binder/scope.hpp"
#include "flowbuilder/flowgraph.hpp"
#include "parser/node.hpp"
#include "parser/nodemap.hpp"

class LoopContext {
public:
    FlowNode* header;
    FlowNode* successor;
};

class FunctionContext {
public:
    FlowNode* successor;
};

class FlowContext {
public:
    std::stack<LoopContext> loopContexts; // stack
    FunctionContext functionContext; // TODO no nested functions for now FIXME
};

using FlowInfo = struct FlowInfo {
    FlowNode* flowNode;
    FlowGraph* flowGraph;
    bool reachable;
};

class FlowBuilderResult {
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
    FlowGraph* buildGraphInternal(Node* node, FlowBuilderResult* result_out);
    FlowNode* buildFlowNode(FlowGraph* graph, Node* node, FlowNode* successor, FlowContext& context, FlowBuilderResult* result_out);
    FlowNode* fNode(Node* node) const;
    void fNode(Node* node, FlowNode* flowNode);
    FlowGraph* fGraph(Node* node) const;
    void fGraph(Node* node, FlowGraph* flowGraph);
    /*bool reachable(Node* node) const;
    void reachable(Node* node, bool reachable);*/
public:
    FlowBuilder();
    std::unique_ptr<FlowBuilderResult> buildGraph(Node* rootNode);
    NodeMap<FlowInfo>* getFlowInfoMap() const;
    std::unique_ptr<NodeMap<FlowInfo>> takeFlowInfoMap();
};