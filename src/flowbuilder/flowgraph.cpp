#include <algorithm>
#include <memory>

#include "flowbuilder/flowgraph.hpp"
#include "flowbuilder/flowbuilder.hpp"
#include "flowbuilder/flownode.hpp"

std::vector<FlowGraph*> FlowBuilderResult::getGraphs() {
    std::vector<FlowGraph*> graphPointers;
    for (auto& graph : this->graphs) {
        graphPointers.push_back(graph.get());
    }
    return graphPointers;
}

void FlowBuilderResult::addGraph(std::unique_ptr<FlowGraph> graph) {
    this->graphs.push_back(std::move(graph));
}

FlowGraph::FlowGraph () {
    auto entry = std::make_unique<FlowNode>(FlowNodeKind::Entry);
    auto exit = std::make_unique<FlowNode>(FlowNodeKind::Exit);
    this->addNode(std::move(entry));
    this->addNode(std::move(exit));
}

void FlowGraph::setAstNode(Node* node) {
    this->astNode = node;
}

Node* FlowGraph::getAstNode() {
    return this->astNode;
}

FlowNode* FlowGraph::getEntry() {
    return this->entry.get();
}

FlowNode* FlowGraph::getExit() {
    return this->exit.get();
}

FlowNode* FlowGraph::addNode(std::unique_ptr<FlowNode> node) {
    if (node->getKind() == FlowNodeKind::Entry) {
        this->entry = std::move(node);
        return this->entry.get();
    }
    if (node->getKind() == FlowNodeKind::Exit) {
        this->exit = std::move(node);
        return this->exit.get();
    }
    if (std::count(this->nodes.begin(), this->nodes.end(), node)) {
        return nullptr;
    }
    this->nodes.push_back(std::move(node));
    return this->nodes.back().get();
}

std::vector<FlowNode*> FlowGraph::getNodes() {
    std::vector<FlowNode*> nodePointers;
    nodePointers.push_back(this->entry.get());
    for (auto& node : this->nodes) {
        nodePointers.push_back(node.get());
    }
    nodePointers.push_back(this->exit.get());
    return nodePointers;
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