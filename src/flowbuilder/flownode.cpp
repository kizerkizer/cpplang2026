#include "flowbuilder/flownode.hpp"
#include "parser/node.hpp"

FlowNode::FlowNode(FlowNodeKind kind) {
    this->kind = kind;
}

int FlowNode::getId () {
    return this->id;
}

Node* FlowNode::getAstNode() {
    return this->astNode;
}

void FlowNode::setAstNode(Node* node) {
    this->astNode = node;
}

std::vector<FlowNode*> FlowNode::getSuccessors() {
    return this->successors;
}

std::vector<FlowNode*> FlowNode::getPredecessors() {
    return this->predecessors;
}

void FlowNode::addSuccessor(FlowNode* successor) {
    this->successors.push_back(successor);
}

void FlowNode::addPredecessor(FlowNode* predecessor) {
    this->predecessors.push_back(predecessor);
}
