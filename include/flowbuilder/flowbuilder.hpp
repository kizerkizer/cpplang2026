#pragma once

#include "binder/scope.hpp"
#include "flower/flowgraph.hpp"

class FlowBuilder {
public:
    FlowBuilder();
    FlowGraph* buildGraph(Node* rootNode, Scope* rootScope);
private:
    FlowGraph* buildGraphInternal(Node* node, Scope* scope, bool atRoot = false);
};