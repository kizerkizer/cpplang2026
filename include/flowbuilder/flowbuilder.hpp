#pragma once

#include <memory>
#include <stack>

#include "binder/scope.hpp"
#include "flowbuilder/flowgraph.hpp"
#include "parser/node.hpp"

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
    FunctionContext functionContext;
};

class FlowBuilderResult {
private:
    std::vector<std::unique_ptr<FlowGraph>> m_graphs;
public:
    std::vector<FlowGraph*> getGraphs();
    void addGraph(std::unique_ptr<FlowGraph> graph);
};

class FlowBuilder {
private:
    FlowGraph* buildGraphInternal(Node* node, FlowBuilderResult* result_out);
    FlowNode* buildFlowNode(FlowGraph* graph, Node* node, FlowNode* successor, FlowContext& context, FlowBuilderResult* result_out);
public:
    FlowBuilder();
    std::unique_ptr<FlowBuilderResult> buildGraph(Node* rootNode);
};