#include "flowbuilder/flownode.hpp"
#include "parser/node.hpp"

int nextId = 0;

std::string flowNodeKindToString(FlowNodeKind kind) {
    switch (kind) {
        case FlowNodeKind::Entry:
            return "Entry";
        case FlowNodeKind::Exit:
            return "Exit";
        case FlowNodeKind::Global:
            return "Global";
        case FlowNodeKind::Block:
            return "Block";
        case FlowNodeKind::Program:
            return "Program";
        case FlowNodeKind::If:
            return "If";
        case FlowNodeKind::Loop:
            return "Loop";
        case FlowNodeKind::Break:
            return "Break";
        case FlowNodeKind::Continue:
            return "Continue";
        case FlowNodeKind::Function:
            return "Function";
        case FlowNodeKind::Return:
            return "Return";
        case FlowNodeKind::Statement:
            return "Statement";
    }
}

FlowNode::FlowNode(FlowNodeKind kind) {
    this->kind = kind;
    this->id = nextId++;
}

int FlowNode::getId () {
    return this->id;
}

void FlowNode::setId(int id) {
    this->id = id;
}

Node* FlowNode::getAstNode() {
    return this->astNode;
}

void FlowNode::setAstNode(Node* node) {
    this->astNode = node;
}

FlowNodeKind FlowNode::getKind() {
    return this->kind;
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
