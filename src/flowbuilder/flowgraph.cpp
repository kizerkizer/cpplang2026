#include <algorithm>
#include <memory>
#include <vector>

#include "flowbuilder/flowgraph.hpp"
#include "flowbuilder/flowbuilder.hpp"
#include "flowbuilder/flownode.hpp"
#include "parser/node.hpp"

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

void _markReachable (FlowNode* node, std::map<FlowNode*, bool>& visited) {
    visited[node] = true;
    for (auto& successor : node->getSuccessors()) {
        if (!visited[successor]) {
            _markReachable(successor, visited);
        }
    }
}

std::vector<FlowNode*> FlowGraph::getUnreachable() {
    std::vector<FlowNode*> nodes = this->getNodes();
    std::map<FlowNode*, bool> visited;
    _markReachable(this->getEntry(), visited);
    std::vector<FlowNode*> unreachableNodes;
    for (auto& node : nodes) {
        if (!visited[node]) {
            unreachableNodes.push_back(node);
        }
    }
    return unreachableNodes;
}

bool FlowGraph::isReachable(FlowNode* start, FlowNode* to) {
    std::map<FlowNode*, bool> visited;
    _markReachable(start, visited);
    return visited[to];
}

void FlowGraph::assignReachabilityToNodes() {
    std::vector<FlowNode*> unreachableNodes = this->getUnreachable();
    std::vector<FlowNode*> nodes = this->getNodes();
    for (auto& node : nodes) {
        if (node->getAstNode() == nullptr) {
            continue;
        }
        if (std::count(unreachableNodes.begin(), unreachableNodes.end(), node)) {
            node->getAstNode()->setReachable(false);
        } else {
            node->getAstNode()->setReachable(true);
        }
    }
}