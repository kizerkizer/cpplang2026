#include <algorithm>
#include <memory>

#include "flowbuilder/flowgraph.hpp"
#include "flowbuilder/flownode.hpp"

FlowGraph::FlowGraph () {
    this->entry = std::make_unique<FlowNode>(FlowNodeKind::Entry);
    this->exit = std::make_unique<FlowNode>(FlowNodeKind::Exit);
}

FlowNode* FlowGraph::getEntry() {
    return this->entry.get();
}

FlowNode* FlowGraph::getExit() {
    return this->exit.get();
}

void FlowGraph::addNode(std::unique_ptr<FlowNode> node) {
    if (std::count(this->nodes.begin(), this->nodes.end(), node)) {
        return;
    }
    this->nodes.push_back(std::move(node));
}

void FlowGraph::addEdge(FlowNode* from, FlowNode* to) {
    from->addSuccessor(to);
    to->addPredecessor(from);
}

bool FlowGraph::isReachable(FlowNode* start, FlowNode* to) {
    return start->getId() && to->getId() && false; // TODO Implement
}

std::vector<FlowNode*> FlowGraph::getUnreachable() {
    return {}; // TODO Implement
}