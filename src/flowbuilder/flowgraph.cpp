#include <algorithm>
#include <memory>
#include <vector>

#include "flowbuilder/flowgraph.hpp"
#include "flowbuilder/flowbuilder.hpp"
#include "flowbuilder/flownode.hpp"
#include "parser/node.hpp"

// FlowBuilderResult
std::vector<FlowGraph*> FlowBuilderResult::getGraphs() {
    std::vector<FlowGraph*> graphPointers;
    for (auto& graph : this->m_graphs) {
        graphPointers.push_back(graph.get());
    }
    return graphPointers;
}

void FlowBuilderResult::addGraph(std::unique_ptr<FlowGraph> graph) {
    this->m_graphs.push_back(std::move(graph));
}

NodeMap<FlowInfo>* FlowBuilderResult::getFlowInfoMap() const {
    return this->m_flowInfoMap.get();
}

void FlowBuilderResult::setFlowInfoMap(std::unique_ptr<NodeMap<FlowInfo>> flowInfoMap) {
    this->m_flowInfoMap = std::move(flowInfoMap);
}

FlowNode* FlowBuilderResult::getFlowNode(Node* node) const {
    auto flowInfo = this->m_flowInfoMap->getValue(node);
    if (flowInfo) {
        return flowInfo->flowNode;
    }
    return nullptr;
}

FlowGraph* FlowBuilderResult::getFlowGraph(Node* node) const {
    auto flowInfo = this->m_flowInfoMap->getValue(node);
    if (flowInfo) {
        return flowInfo->flowGraph;
    }
    return nullptr;
}

// FlowGraph
FlowGraph::FlowGraph () {
    m_flowGraphNodeMap = std::make_unique<NodeMap<FlowGraphInfo>>();
    auto entry = std::make_unique<FlowNode>(FlowNodeKind::Entry);
    auto exit = std::make_unique<FlowNode>(FlowNodeKind::Exit);
    this->addNode(std::move(entry));
    this->addNode(std::move(exit));
}

void FlowGraph::setReachable(Node* node, bool reachable) {
    auto flowInfo = this->m_flowGraphNodeMap->getValue(node);
    if (flowInfo) {
        flowInfo->reachable = reachable;
    } else {
        auto newFlowInfo = std::make_unique<FlowGraphInfo>();
        newFlowInfo->reachable = reachable;
        this->m_flowGraphNodeMap->setValue(node, std::move(newFlowInfo));
    }
}

bool FlowGraph::getReachable(Node* node) {
    auto flowInfo = this->m_flowGraphNodeMap->getValue(node);
    if (flowInfo) {
        return flowInfo->reachable;
    }
    return false; // default to false if not set
}

NodeMap<FlowGraphInfo>* FlowGraph::getFlowGraphNodeMap() const {
    return this->m_flowGraphNodeMap.get();
}

std::unique_ptr<NodeMap<FlowGraphInfo>> FlowGraph::takeFlowGraphNodeMap() {
    return std::move(this->m_flowGraphNodeMap);
}

void FlowGraph::setAstNode(Node* node) {
    this->m_astNode = node;
}

Node* FlowGraph::getAstNode() {
    return this->m_astNode;
}

FlowNode* FlowGraph::getEntry() {
    return this->m_entry.get();
}

FlowNode* FlowGraph::getExit() {
    return this->m_exit.get();
}

FlowNode* FlowGraph::addNode(std::unique_ptr<FlowNode> node) {
    if (node->getKind() == FlowNodeKind::Entry) {
        this->m_entry = std::move(node);
        return this->m_entry.get();
    }
    if (node->getKind() == FlowNodeKind::Exit) {
        this->m_exit = std::move(node);
        return this->m_exit.get();
    }

    // If already added, return nullptr TODO ??
    if (std::count(this->m_nodes.begin(), this->m_nodes.end(), node)) {
        return nullptr;
    }
    this->m_nodes.push_back(std::move(node));
    return this->m_nodes.back().get();
}

std::vector<FlowNode*> FlowGraph::getNodes() {
    std::vector<FlowNode*> nodePointers;
    nodePointers.push_back(this->m_entry.get());
    for (auto& node : this->m_nodes) {
        nodePointers.push_back(node.get());
    }
    nodePointers.push_back(this->m_exit.get());
    return nodePointers;
}

void FlowGraph::addEdge(FlowNode* from, FlowNode* to, std::optional<std::string> edgeName) {
    from->addSuccessor(to, edgeName);
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
    return visited.count(to) > 0;
}

bool FlowGraph::isASTNodeReachable(Node* node) {
    auto flowInfo = this->m_flowGraphNodeMap->getValue(node);
    if (flowInfo) {
        return flowInfo->reachable;
    }
    return false; // default to false if not set
}

void FlowGraph::assignReachabilityToNodes() {
    std::vector<FlowNode*> unreachableNodes = this->getUnreachable();
    std::vector<FlowNode*> nodes = this->getNodes();
    for (auto& node : nodes) {
        if (node->getAstNode() == nullptr) {
            continue;
        }
        if (std::count(unreachableNodes.begin(), unreachableNodes.end(), node)) {
            setReachable(node->getAstNode(), false);
        } else {
            setReachable(node->getAstNode(), true);
        }
    }
}