#pragma once

#include <vector>

class Node; // defined in parser/node.hpp

enum class FlowNodeKind {
    Entry,
    Exit,
    Global,
    Block,
    Program,
    If,
    Loop,
    Break,
    Continue,
    Function,
    Return,
    Statement,
};

std::string flowNodeKindToString(FlowNodeKind kind);

class FlowNode {
private:
    FlowNodeKind kind;
    int id = 0;
    Node* astNode = nullptr;
    std::vector<FlowNode*> successors;
    std::vector<FlowNode*> predecessors;
public:
    FlowNode(FlowNodeKind kind);
    FlowNodeKind getKind();
    int getId();
    void setId(int id);
    Node* getAstNode();
    void setAstNode(Node* node);
    std::vector<FlowNode*> getSuccessors();
    std::vector<FlowNode*> getPredecessors();
    void addSuccessor(FlowNode* successor);
    void addPredecessor(FlowNode* predecessor);
};