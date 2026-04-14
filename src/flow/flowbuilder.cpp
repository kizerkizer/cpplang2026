#include <cstdlib>
#include <memory>
#include <print>
#include <cassert>

#include "flow/flowbuilder.hpp"
#include "diagnostics/diagnostics.hpp"
#include "flow/flownode.hpp"
#include "parser/node.hpp"
#include "common/util.hpp"

// FlowBuilderResult
std::vector<FlowGraph*> FlowAnalyzerResult::getGraphs() {
    std::vector<FlowGraph*> graphPointers;
    for (auto& graph : this->m_graphs) {
        graphPointers.push_back(graph.get());
    }
    return graphPointers;
}

void FlowAnalyzerResult::addGraph(std::unique_ptr<FlowGraph> graph) {
    this->m_graphs.push_back(std::move(graph));
}

NodeMap<FlowInfo>* FlowAnalyzerResult::getFlowInfoMap() const {
    return this->m_flowInfoMap.get();
}

void FlowAnalyzerResult::setFlowInfoMap(std::unique_ptr<NodeMap<FlowInfo>> flowInfoMap) {
    this->m_flowInfoMap = std::move(flowInfoMap);
}

FlowNode* FlowAnalyzerResult::getFlowNode(Node* node) const {
    auto flowInfo = this->m_flowInfoMap->getValue(node);
    if (flowInfo) {
        return flowInfo->flowNode;
    }
    return nullptr;
}

FlowGraph* FlowAnalyzerResult::getFlowGraph(Node* node) const {
    auto flowInfo = this->m_flowInfoMap->getValue(node);
    if (flowInfo) {
        return flowInfo->flowGraph;
    }
    return nullptr;
}

// FlowBuilder
FlowBuilder::FlowBuilder(Diagnostics &diagnostics) : m_diagnostics(diagnostics) {
    m_flowInfoMap = std::make_unique<NodeMap<FlowInfo>>();
}

FlowNode* FlowBuilder::getFlowNode(Node* node) const {
    auto flowInfo = this->m_flowInfoMap->getValue(node);
    if (flowInfo) {
        return flowInfo->flowNode;
    }
    return nullptr;
}

void FlowBuilder::setFlowNode(Node* node, FlowNode* flowNode) {
    auto flowInfo = this->m_flowInfoMap->getValue(node);
    if (flowInfo) {
        flowInfo->flowNode = flowNode;
    } else {
        auto newFlowInfo = std::make_unique<FlowInfo>();
        newFlowInfo->flowNode = flowNode;
        this->m_flowInfoMap->setValue(node, std::move(newFlowInfo));
    }
}

FlowGraph* FlowBuilder::getFlowGraph(Node* node) const {
    auto flowInfo = this->m_flowInfoMap->getValue(node);
    if (flowInfo) {
        return flowInfo->flowGraph;
    }
    return nullptr;
}

void FlowBuilder::setFlowGraph(Node* node, FlowGraph* flowGraph) {
    auto flowInfo = this->m_flowInfoMap->getValue(node);
    if (flowInfo) {
        flowInfo->flowGraph = flowGraph;
    } else {
        auto newFlowInfo = std::make_unique<FlowInfo>();
        newFlowInfo->flowGraph = flowGraph;
        this->m_flowInfoMap->setValue(node, std::move(newFlowInfo));
    }
}

std::unique_ptr<FlowAnalyzerResult> FlowBuilder::buildGraph(Node* rootNode) {
    auto result = std::make_unique<FlowAnalyzerResult>();
    this->buildGraphInternal(rootNode, result.get());
    result->setFlowInfoMap(this->takeFlowInfoMap());
    return result;
}

std::unique_ptr<NodeMap<FlowInfo>> FlowBuilder::takeFlowInfoMap() {
    return std::move(this->m_flowInfoMap);
}

FlowNode* FlowBuilder::buildFlowNode(FlowGraph* graph, Node* node, FlowNode* successor, FlowContext& context, FlowAnalyzerResult* result) {
    switch (node->getNodeKind()) {
        case NodeKind::FunctionDeclaration: {
            auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Function);
            flowNode->setAstNode(node);
            setFlowNode(node, flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            graph->addEdge(flowNodePointer, successor);
            auto builtFlowGraph = buildGraphInternal(node, result);
            setFlowGraph(functionDeclarationNode, builtFlowGraph);
            builtFlowGraph->setAstNode(node); //
            return flowNodePointer;
        }
        case NodeKind::IfStatement: {
            auto ifStatementNode = static_cast<IfStatementNode*>(node);
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::If);
            flowNode->setAstNode(node);
            setFlowNode(node, flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            auto thenFlowNode = buildFlowNode(graph, ifStatementNode->getThenBranch(), successor, context, result);
            auto elseFlowNode = successor;
            if (ifStatementNode->getElseBranch()) {
                elseFlowNode = buildFlowNode(graph, ifStatementNode->getElseBranch(), successor, context, result);
            }
            graph->addEdge(flowNodePointer, thenFlowNode, "then");
            graph->addEdge(flowNodePointer, elseFlowNode, "else");
            return flowNodePointer;
        }
        case NodeKind::LoopStatement: {
            auto loopStatementNode = static_cast<LoopStatementNode*>(node);
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Loop);
            flowNode->setAstNode(node);
            setFlowNode(node, flowNode.get());
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

            // Link to loop successor
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Break);
            flowNode->setAstNode(node);
            setFlowNode(node, flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            LoopContext loopContext = context.loopContexts.top();
            auto loopSuccessor = loopContext.successor;
            graph->addEdge(flowNodePointer, loopSuccessor);
            return flowNodePointer;
        }
        case NodeKind::ContinueStatement: {

            // Link to loop header
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Continue);
            flowNode->setAstNode(node);
            setFlowNode(node, flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            LoopContext loopContext = context.loopContexts.top();
            auto loopHeader = loopContext.header;
            graph->addEdge(flowNodePointer, loopHeader);
            return flowNodePointer;
        }
        case NodeKind::ReturnStatement: {

            // Link to function successor
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Return);
            flowNode->setAstNode(node);
            setFlowNode(node, flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            auto functionSuccessor = context.functionContext.successor;
            graph->addEdge(flowNodePointer, functionSuccessor);
            return flowNodePointer;
        }
        case NodeKind::BlockStatement: {
            BlockStatementNode* blockStatementNode = static_cast<BlockStatementNode*>(node);
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Block);
            flowNode->setAstNode(node);
            setFlowNode(node, flowNode.get());
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
            auto children = executionListNode->getChildren();
            auto next = successor;
            for (int i = children.size() - 1; i >= 0; --i) {
                next = buildFlowNode(graph, children[i], next, context, result);
            }
            return next;
        }
        default: {
            std::unique_ptr<FlowNode> flowNode = std::make_unique<FlowNode>(FlowNodeKind::Statement);
            flowNode->setAstNode(node);
            setFlowNode(node, flowNode.get());
            auto flowNodePointer = graph->addNode(std::move(flowNode));
            graph->addEdge(flowNodePointer, successor);
            return flowNodePointer;
        }
    }
}

FlowGraph* FlowBuilder::buildGraphInternal(Node* node, FlowAnalyzerResult* result_out) {
    Node* nodeToBuild = nullptr;
    std::unique_ptr<FlowNode> flowNode = nullptr;
    auto graph = std::make_unique<FlowGraph>();
    auto entry = graph->getEntry();
    auto exit = graph->getExit();
    auto flowContext = FlowContext();

    // Either the global scope, or a function
    if (node->getNodeKind() == NodeKind::Program) {
        auto programNode = static_cast<ProgramNode*>(node);
        setFlowGraph(programNode, graph.get());
        graph->setAstNode(node);
        flowNode = std::make_unique<FlowNode>(FlowNodeKind::Global);
        nodeToBuild = programNode;
    } else if (node->getNodeKind() == NodeKind::FunctionDeclaration) {
        auto functionNode = static_cast<FunctionDeclarationNode*>(node);
        setFlowGraph(functionNode, graph.get());
        flowNode = std::make_unique<FlowNode>(FlowNodeKind::Function);
        flowContext.functionContext.successor = exit;
        graph->setAstNode(node);
        nodeToBuild = functionNode->getBody();
    } else {
        unreachable();
        return nullptr;
    }
    flowNode->setAstNode(node);
    FlowNode* builtFlowNode = buildFlowNode(graph.get(), nodeToBuild, exit, flowContext, result_out);
    graph->addEdge(entry, builtFlowNode);
    auto graphPointer = graph.get();
    result_out->addGraph(std::move(graph));
    return graphPointer;
}