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

constexpr const char* flowNodeKindToString(FlowNodeKind kind) {
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

class FlowNode {
private:
    FlowNodeKind m_kind;
    int m_id = 0;
    Node* m_astNode = nullptr;
    std::vector<FlowNode*> m_successors;
    std::vector<FlowNode*> m_predecessors;
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