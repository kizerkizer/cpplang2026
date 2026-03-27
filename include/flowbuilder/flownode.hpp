#pragma once

#include <vector>

class Node; // defined in parser/node.hpp

enum class FlowNodeKind {
    Entry,
    Exit,
    Global,
    Block,
    If,
    Loop,
    Break,
    Continue,
    Function,
    Return,
    Statement,
};

class FlowNode {
private:
    FlowNodeKind kind;
    int id = 0;
    Node* astNode;
    std::vector<FlowNode*> successors;
    std::vector<FlowNode*> predecessors;
public:
    FlowNode(FlowNodeKind kind);
    FlowNodeKind getKind();
    int getId();
    Node* getAstNode();
    void setAstNode(Node* node);
    std::vector<FlowNode*> getSuccessors();
    std::vector<FlowNode*> getPredecessors();
    void addSuccessor(FlowNode* successor);
    void addPredecessor(FlowNode* predecessor);
};