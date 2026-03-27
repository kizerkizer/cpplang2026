#include "flowbuilder/flowbuilder.hpp"
#include "flowbuilder/flownode.hpp"
#include "parser/node.hpp"

FlowBuilder::FlowBuilder() {

}

FlowGraph* FlowBuilder::buildGraph(Node* rootNode, Scope* rootScope) {
    return this->buildGraphInternal(rootNode, rootScope, true);
}

FlowGraph* FlowBuilder::buildGraphInternal(Node* node, Scope* scope, bool atRoot) {
    ProgramNode* programNode = nullptr;
    std::unique_ptr<FlowNode> flowNode = nullptr;
    if (atRoot) {
        programNode = static_cast<ProgramNode*>(node);
        flowNode = std::make_unique<FlowNode>(FlowNodeKind::Global);
    } else {
        auto functionNode = static_cast<FunctionDeclarationNode*>(node);
        programNode = functionNode->getBody()->getProgramNode();
        flowNode = std::make_unique<FlowNode>(FlowNodeKind::Function);
    }
    flowNode->setAstNode(node);
    for (const auto child : programNode->getChildren()) {

    }
    switch (node->getNodeKind()) {
        /*case NodeKind::Program: {
            auto flowNode = std::make_unique<FlowNode>(atRoot ? FlowNodeKind::Global : FlowNodeKind::Block);
            flowNode->setAstNode(node);
        }*/
        default: {
            auto flowNode = std::make_unique<FlowNode>(FlowNodeKind::Statement);
            break;
        }
    }
    return nullptr;

}