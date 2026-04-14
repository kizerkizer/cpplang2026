#include "flow/flownode.hpp"
#include "parser/node.hpp"
#include "common/programuniqueid.hpp"

// FlowNode
FlowNode::FlowNode(FlowNodeKind kind) {
    this->m_kind = kind;
    this->m_id = getNextId();
}

int FlowNode::getId () {
    return this->m_id;
}

void FlowNode::setId(int id) {
    this->m_id = id;
}

Node* FlowNode::getAstNode() {
    return this->m_astNode;
}

void FlowNode::setAstNode(Node* node) {
    this->m_astNode = node;
}

FlowNodeKind FlowNode::getKind() {
    return this->m_kind;
}

std::vector<FlowNode*> FlowNode::getSuccessors() {
    return this->m_successors;
}

std::vector<FlowNode*> FlowNode::getPredecessors() {
    return this->m_predecessors;
}

void FlowNode::addSuccessor(FlowNode* successor, std::optional<std::string> edgeName) {
    this->m_successors.push_back(successor);
    if (edgeName.has_value()) {
        this->m_successorsByEdgeName[edgeName.value()] = successor;
    }
}

void FlowNode::addPredecessor(FlowNode* predecessor) {
    this->m_predecessors.push_back(predecessor);
}

FlowNode* FlowNode::getSuccessorByEdgeName(const std::string& edgeName) {
    auto it = this->m_successorsByEdgeName.find(edgeName);
    if (it != this->m_successorsByEdgeName.end()) {
        return it->second;
    }
    return nullptr;
}