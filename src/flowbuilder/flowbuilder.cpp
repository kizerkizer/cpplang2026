#include <cstdlib>
#include <memory>
#include <print>
#include <cassert>

#include "flowbuilder/flowbuilder.hpp"
#include "flowbuilder/flownode.hpp"
#include "parser/node.hpp"

FlowBuilder::FlowBuilder() {

}

std::unique_ptr<FlowBuilderResult> FlowBuilder::buildGraph(Node* rootNode) {
    auto result = std::make_unique<FlowBuilderResult>();
    this->buildGraphInternal(rootNode, result.get());
    return result;
}

FlowNode* FlowBuilder::buildFlowNode(FlowGraph* graph, Node* node, FlowNode* successor, FlowContext& context, FlowBuilderResult* result) {
    switch (node->getNodeKind()) {
        case NodeKind::FunctionDeclaration: {
            auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Function);
            flowNode->setAstNode(node);
            node->setFlowNode(flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            graph->addEdge(flowNodePointer, successor);
            auto builtFlowGraph = buildGraphInternal(node, result);
            functionDeclarationNode->setFlowGraph(builtFlowGraph);
            //builtFlowGraph->setAstNode(node); // TODO uncomment once other memories errors fixed
            return flowNodePointer;
        }
        case NodeKind::IfStatement: {
            auto ifStatementNode = static_cast<IfStatementNode*>(node);
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::If);
            flowNode->setAstNode(node);
            node->setFlowNode(flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            auto thenFlowNode = buildFlowNode(graph, ifStatementNode->getThenBranch(), successor, context, result);
            auto elseFlowNode = successor;
            if (ifStatementNode->getElseBranch()) {
                elseFlowNode = buildFlowNode(graph, ifStatementNode->getElseBranch(), successor, context, result);
            }
            graph->addEdge(flowNodePointer, thenFlowNode);
            graph->addEdge(flowNodePointer, elseFlowNode);
            return flowNodePointer;
        }
        case NodeKind::LoopStatement: {
            auto loopStatementNode = static_cast<LoopStatementNode*>(node);
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Loop);
            flowNode->setAstNode(node);
            node->setFlowNode(flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            auto loopContext = LoopContext();
            loopContext.header = flowNodePointer;
            loopContext.successor = successor;
            context.loopContexts.push(loopContext);
            FlowNode* loopFlowNode = buildFlowNode(graph, loopStatementNode->getBody(), loopContext.header, context, result);
            context.loopContexts.pop();
            graph->addEdge(flowNodePointer, loopFlowNode);
            return flowNodePointer;
        }
        case NodeKind::BreakStatement: {
            // link to loop successor
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Break);
            flowNode->setAstNode(node);
            node->setFlowNode(flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            LoopContext loopContext = context.loopContexts.top();
            auto loopSuccessor = loopContext.successor;
            graph->addEdge(flowNodePointer, loopSuccessor);
            return flowNodePointer;
        }
        case NodeKind::ContinueStatement: {
            // link to loop header
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Continue);
            flowNode->setAstNode(node);
            node->setFlowNode(flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            LoopContext loopContext = context.loopContexts.top();
            auto loopHeader = loopContext.header;
            graph->addEdge(flowNodePointer, loopHeader);
            return flowNodePointer;
        }
        case NodeKind::ReturnStatement: {
            // link to function successor
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Return);
            flowNode->setAstNode(node);
            node->setFlowNode(flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            auto functionSuccessor = context.functionContext.successor;
            graph->addEdge(flowNodePointer, functionSuccessor);
            return flowNodePointer;
        }
        case NodeKind::BlockStatement: {
            BlockStatementNode* blockStatementNode = static_cast<BlockStatementNode*>(node);
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Block);
            flowNode->setAstNode(node);
            node->setFlowNode(flowNode.get());
            FlowNode* blockStatementNodePointer = graph->addNode(std::move(flowNode));
            auto executionListNode = blockStatementNode->getExecutionListNode();
            auto next = successor;
            auto children = executionListNode->getChildren();
            for (int i = children.size() - 1; i >= 0; --i) {
                next = buildFlowNode(graph, children[i], next, context, result);
            }
            graph->addEdge(blockStatementNodePointer, next);
            return blockStatementNodePointer;
        }
        case NodeKind::Program: {
            auto programNode = static_cast<ProgramNode*>(node);
            auto executionListNode = programNode->getExecutionListNode();

            //std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Global);
            //flowNode->setAstNode(node);
            //node->setFlowNode(flowNode.get());
            //auto flowNodePointer = graph->addNode(std::move(flowNode));
            auto children = executionListNode->getChildren();
            auto next = successor;
            for (int i = children.size() - 1; i >= 0; --i) {
                next = buildFlowNode(graph, children[i], next, context, result);
            }
            //graph->addEdge(flowNodePointer, next); // next is now first flownode within the block
            return next;
        }
        default: {
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Statement);
            flowNode->setAstNode(node);
            node->setFlowNode(flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            graph->addEdge(flowNodePointer, successor);
            return flowNodePointer;
        }
    }
}

FlowGraph* FlowBuilder::buildGraphInternal(Node* node, FlowBuilderResult* result) {
    Node* nodeToBuild = nullptr;
    std::unique_ptr<FlowNode> flowNode = nullptr;
    auto graph = std::make_unique<FlowGraph>();
    auto entry = graph->getEntry();
    auto exit = graph->getExit();
    auto flowContext = FlowContext();
    // Either the global scope, or a function:
    if (node->getNodeKind() == NodeKind::Program) {
        auto programNode = static_cast<ProgramNode*>(node);
        programNode->setFlowGraph(graph.get());
        graph->setAstNode(node);
        flowNode = std::make_unique<FlowNode>(FlowNodeKind::Global);
        nodeToBuild = programNode;
    } else if (node->getNodeKind() == NodeKind::FunctionDeclaration) {
        auto functionNode = static_cast<FunctionDeclarationNode*>(node);
        functionNode->setFlowGraph(graph.get());
        flowNode = std::make_unique<FlowNode>(FlowNodeKind::Function);
        flowContext.functionContext.successor = exit;
        graph->setAstNode(node);
        nodeToBuild = functionNode->getBody();
    } else {
        std::print("Error: bad node kind given to buildGraphInternal. NodeKind={}\n", nodeKindToString(node->getNodeKind()));
        return nullptr;
    }
    flowNode->setAstNode(node);
    FlowNode* programFlowNode = buildFlowNode(graph.get(), nodeToBuild, exit, flowContext, result);
    graph->addEdge(entry, programFlowNode);
    auto graphPointer = graph.get();
    result->addGraph(std::move(graph));
    return graphPointer;
}