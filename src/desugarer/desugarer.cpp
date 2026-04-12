#include <memory>

#include "desugarer/desugarer.hpp"
#include "common/sourcecodelocation.hpp"
#include "parser/node.hpp"
#include "common/util.hpp"

template <typename To, typename From>
std::unique_ptr<To> unique_ptr_static_cast(std::unique_ptr<From> from) {
    return std::unique_ptr<To>(static_cast<To*>(from.release()));
}

std::unique_ptr<Node> Desugarer::desugarWhileStatement(std::unique_ptr<Node> node) {
    ASSERT(node->getNodeKind() == NodeKind::WhileStatement);
    auto* whileStatementNode = static_cast<WhileStatementNode*>(node.get());
    
    // First, desugar the condition and body of the while statement
    whileStatementNode->setCondition(unique_ptr_static_cast<ExpressionNode>(desugarRecursive(whileStatementNode->takeCondition())));
    whileStatementNode->setBody(unique_ptr_static_cast<BlockStatementNode>(desugarRecursive(whileStatementNode->takeBody())));

    // Desugar `while (condition) body;` into:
    // loop {
    //     if (!condition) {
    //         break;
    //     }
    //     body
    // }
    // TODO reference original source code locations
    auto ifThenBlockExecutionListNode = std::make_unique<ExecutionListNode>(englishbreakfast::emptySourceCodeLocationSpan);
    ifThenBlockExecutionListNode->addNode(std::make_unique<BreakStatementNode>(englishbreakfast::emptySourceCodeLocationSpan));
    auto ifThenBlockStatementNode = std::make_unique<BlockStatementNode>(std::move(ifThenBlockExecutionListNode), englishbreakfast::emptySourceCodeLocationSpan);
    auto newIfStatement = std::make_unique<IfStatementNode>(
        std::make_unique<UnaryOperatorExpressionNode>(
            whileStatementNode->takeCondition(),
            std::make_unique<Token>(nullptr, "!", englishbreakfast::emptySourceCodeLocationSpan, TokenKind::Not, true),
            englishbreakfast::emptySourceCodeLocationSpan
        ),
        std::move(ifThenBlockStatementNode),
        nullptr,
        englishbreakfast::emptySourceCodeLocationSpan
    );
    auto newExecutionListNode = std::make_unique<ExecutionListNode>(englishbreakfast::emptySourceCodeLocationSpan);
    newExecutionListNode->addNode(std::move(newIfStatement));
    auto children = whileStatementNode->takeBody()->takeExecutionListNode()->takeChildren();
    for (auto& child : children) {
        newExecutionListNode->addNode(std::move(child));
    }
    auto newBody = std::make_unique<BlockStatementNode>(std::move(newExecutionListNode), englishbreakfast::emptySourceCodeLocationSpan);
    auto loopStatement = std::make_unique<LoopStatementNode>(std::move(newBody), englishbreakfast::emptySourceCodeLocationSpan);
    return loopStatement;
}

std::unique_ptr<Node> Desugarer::desugarRecursive(std::unique_ptr<Node> node) {
    if (node == nullptr) {
        return nullptr;
    }
    switch (node->getNodeKind()) {
        case NodeKind::WhileStatement: return desugarWhileStatement(std::move(node));
        case NodeKind::ExecutionList: {
            auto* executionListNode = static_cast<ExecutionListNode*>(node.get());
            std::vector<std::unique_ptr<Node>> newChildren;
            for (auto& child : executionListNode->takeChildren()) {
                newChildren.push_back(desugarRecursive(std::move(child)));
            }
            executionListNode->setChildren(std::move(newChildren));
            return unique_ptr_static_cast<ExecutionListNode>(std::move(node));
        }
        case NodeKind::Program: {
            auto* programNode = static_cast<ProgramNode*>(node.get());
            auto executionListNode = programNode->takeExecutionListNode();
            std::vector<std::unique_ptr<Node>> newChildren;
            for (auto& child : executionListNode->takeChildren()) {
                newChildren.push_back(desugarRecursive(std::move(child)));
            }
            executionListNode->setChildren(std::move(newChildren));
            programNode->setExecutionListNode(std::move(executionListNode));
            return unique_ptr_static_cast<ProgramNode>(std::move(node));
        }
        case NodeKind::VariableDeclaration: {
            auto* variableDeclarationNode = static_cast<VariableDeclarationNode*>(node.get());
            variableDeclarationNode->setIdentifier(unique_ptr_static_cast<IdentifierNode>(desugarRecursive(variableDeclarationNode->takeIdentifier())));
            variableDeclarationNode->setExpression(unique_ptr_static_cast<ExpressionNode>(desugarRecursive(variableDeclarationNode->takeExpression())));
            variableDeclarationNode->setTypeAnnotation(unique_ptr_static_cast<Node>(desugarRecursive(variableDeclarationNode->takeTypeAnnotation())));
            return unique_ptr_static_cast<VariableDeclarationNode>(std::move(node));
        }
        case NodeKind::AssignmentExpression: {
            auto* assignmentExpressionNode = static_cast<AssignmentExpressionNode*>(node.get());
            std::unique_ptr<IdentifierNode> identifierNode = unique_ptr_static_cast<IdentifierNode>(desugarRecursive(assignmentExpressionNode->takeIdentifier()));
            std::unique_ptr<ExpressionNode> expressionNode = unique_ptr_static_cast<ExpressionNode>(desugarRecursive(assignmentExpressionNode->takeExpression()));
            assignmentExpressionNode->setIdentifier(std::move(identifierNode));
            assignmentExpressionNode->setExpression(std::move(expressionNode));
            return unique_ptr_static_cast<AssignmentExpressionNode>(std::move(node));
        }
        case NodeKind::BlockStatement: {
            auto* blockStatementNode = static_cast<BlockStatementNode*>(node.get());
            blockStatementNode->setExecutionListNode(unique_ptr_static_cast<ExecutionListNode>(desugarRecursive(blockStatementNode->takeExecutionListNode())));
            return unique_ptr_static_cast<BlockStatementNode>(std::move(node));
        }
        case NodeKind::FunctionDeclaration: {
            auto* functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node.get());
            functionDeclarationNode->setIdentifier(unique_ptr_static_cast<IdentifierNode>(desugarRecursive(functionDeclarationNode->takeIdentifier())));
            std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> newParameters;
            for (auto& parameter : functionDeclarationNode->takeParameters()) {
                newParameters.push_back(unique_ptr_static_cast<IdentifierWithPossibleAnnotationNode>(desugarRecursive(std::move(parameter))));
            }
            functionDeclarationNode->setParameters(std::move(newParameters));
            functionDeclarationNode->setBodyNode(unique_ptr_static_cast<BlockStatementNode>(desugarRecursive(functionDeclarationNode->takeBodyNode())));
            return unique_ptr_static_cast<FunctionDeclarationNode>(std::move(node));
        }
        case NodeKind::IfStatement: {
            auto* ifStatementNode = static_cast<IfStatementNode*>(node.get());
            ifStatementNode->setCondition(unique_ptr_static_cast<ExpressionNode>(desugarRecursive(ifStatementNode->takeCondition())));
            ifStatementNode->setThenBranch(desugarRecursive(ifStatementNode->takeThenBranch()));
            if (ifStatementNode->getElseBranch()) {
                ifStatementNode->setElseBranch(desugarRecursive(ifStatementNode->takeElseBranch()));
            }
            return unique_ptr_static_cast<IfStatementNode>(std::move(node));
        }
        case NodeKind::LoopStatement: {
            auto* loopStatementNode = static_cast<LoopStatementNode*>(node.get());
            loopStatementNode->setBody(unique_ptr_static_cast<BlockStatementNode>(desugarRecursive(loopStatementNode->takeBody())));
            return unique_ptr_static_cast<LoopStatementNode>(std::move(node));
        }
        case NodeKind::ReturnStatement: {
            auto* returnStatementNode = static_cast<ReturnStatementNode*>(node.get());
            if (returnStatementNode->getExpression()) {
                returnStatementNode->setExpression(unique_ptr_static_cast<ExpressionNode>(desugarRecursive(returnStatementNode->takeExpression())));
            }
            return unique_ptr_static_cast<ReturnStatementNode>(std::move(node));
        }
        case NodeKind::AssignmentStatement: {
            auto* assignmentStatementNode = static_cast<AssignmentStatementNode*>(node.get());
            assignmentStatementNode->setAssignmentExpression(unique_ptr_static_cast<AssignmentExpressionNode>(desugarRecursive(assignmentStatementNode->takeAssignmentExpression())));
            return unique_ptr_static_cast<AssignmentStatementNode>(std::move(node));
        }
        case NodeKind::FunctionCallExpression: {
            auto* functionCallExpressionNode = static_cast<FunctionCallExpressionNode*>(node.get());
            functionCallExpressionNode->setIdentifier(unique_ptr_static_cast<IdentifierNode>(desugarRecursive(functionCallExpressionNode->takeIdentifier())));
            std::vector<std::unique_ptr<ExpressionNode>> newArgumentNodes;
            for (auto& argument : functionCallExpressionNode->takeArgumentNodes()) {
                newArgumentNodes.push_back(unique_ptr_static_cast<ExpressionNode>(desugarRecursive(std::move(argument))));
            }
            functionCallExpressionNode->setArgumentNodes(std::move(newArgumentNodes));
            return unique_ptr_static_cast<FunctionCallExpressionNode>(std::move(node));
        }
        case NodeKind::FunctionCallStatement: {
            auto* functionCallStatementNode = static_cast<FunctionCallStatementNode*>(node.get());
            functionCallStatementNode->setFunctionCallExpression(unique_ptr_static_cast<FunctionCallExpressionNode>(desugarRecursive(functionCallStatementNode->takeFunctionCallExpression())));
            return unique_ptr_static_cast<FunctionCallStatementNode>(std::move(node));
        }
        case NodeKind::BinaryOperatorExpression: {
            auto* binaryOperatorExpressionNode = static_cast<BinaryOperatorExpressionNode*>(node.get());
            binaryOperatorExpressionNode->setLeft(unique_ptr_static_cast<ExpressionNode>(desugarRecursive(binaryOperatorExpressionNode->takeLeft())));
            binaryOperatorExpressionNode->setRight(unique_ptr_static_cast<ExpressionNode>(desugarRecursive(binaryOperatorExpressionNode->takeRight())));
            return unique_ptr_static_cast<BinaryOperatorExpressionNode>(std::move(node));
        }
        case NodeKind::UnaryOperatorExpression: {
            auto* unaryOperatorExpressionNode = static_cast<UnaryOperatorExpressionNode*>(node.get());
            unaryOperatorExpressionNode->setOperand(unique_ptr_static_cast<ExpressionNode>(desugarRecursive(unaryOperatorExpressionNode->takeOperand())));
            return unique_ptr_static_cast<UnaryOperatorExpressionNode>(std::move(node));
        }
        case NodeKind::IfExpression: {
            auto* ifExpressionNode = static_cast<IfExpressionNode*>(node.get());
            ifExpressionNode->setCondition(unique_ptr_static_cast<ExpressionNode>(desugarRecursive(ifExpressionNode->takeCondition())));
            ifExpressionNode->setThenBranch(unique_ptr_static_cast<ExpressionNode>(desugarRecursive(ifExpressionNode->takeThenBranch())));
            if (ifExpressionNode->getElseBranch()) {
                ifExpressionNode->setElseBranch(unique_ptr_static_cast<ExpressionNode>(desugarRecursive(ifExpressionNode->takeElseBranch())));
            }
            return unique_ptr_static_cast<IfExpressionNode>(std::move(node));
        }

        // Un-desugarable leaves:
        case NodeKind::Identifier:
        case NodeKind::IdentifierWithPossibleAnnotation:
        case NodeKind::TypePrimitive:
        case NodeKind::BinaryOperatorTypeExpression:
        case NodeKind::TypeIdentifier:
        case NodeKind::TypeDeclaration:
        case NodeKind::IntegerLiteral:
        case NodeKind::FloatLiteral:
        case NodeKind::StringLiteral:
        case NodeKind::BooleanLiteral:
        case NodeKind::EmptyLiteral:
        case NodeKind::BreakStatement:
        case NodeKind::ContinueStatement:
        case NodeKind::Invalid:
            return node;
    }
}

// Desugarer
std::unique_ptr<Node> Desugarer::desugar() {
    this->m_root = this->desugarRecursive(std::move(this->m_root));
    return std::move(this->m_root);
}