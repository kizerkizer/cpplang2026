#include "desugarer/desugarer.hpp"
#include "common/sourcecodelocation.hpp"
#include "parser/node.hpp"
#include <memory>

template <typename To, typename From>
std::unique_ptr<To> unique_ptr_static_cast(std::unique_ptr<From> from) {
    return std::unique_ptr<To>(static_cast<To*>(from.release()));
}

std::unique_ptr<Node> _desugar (std::unique_ptr<Node> node) {
    switch (node->getNodeKind()) {
        case NodeKind::WhileStatement: {
            auto* whileStatementNode = static_cast<WhileStatementNode*>(node.get());
            whileStatementNode->setCondition(unique_ptr_static_cast<ExpressionNode>(_desugar(whileStatementNode->takeCondition())));
            whileStatementNode->setBody(unique_ptr_static_cast<BlockStatementNode>(_desugar(whileStatementNode->takeBody())));
            // Desugar while(condition) body; into:
            // loop {
            //     if (!condition) {
            //         break;
            //     }
            //     body
            // }
            // TODO should we reference original source code locations? Or just keep using -1,-1,-1 for compiler-created nodes?
            auto newProgramNode = std::make_unique<ProgramNode>(emptySourceCodeLocationSpan, false);
            auto newIfStatement = std::make_unique<IfStatementNode>(
                std::make_unique<UnaryOperatorExpressionNode>(
                    whileStatementNode->takeCondition(),
                    std::make_unique<Token>(nullptr, "!", emptySourceCodeLocationSpan, TokenKind::Not, true),
                    emptySourceCodeLocationSpan
                ),
                std::make_unique<BreakStatementNode>(emptySourceCodeLocationSpan), // TODO should be blockstatement, unless statements in general allowed
                nullptr,
                emptySourceCodeLocationSpan
            );
            newProgramNode->addNode(std::move(newIfStatement));
            auto children = whileStatementNode->takeBody()->takeProgramNode()->takeChildren();
            for (auto& child : children) {
                newProgramNode->addNode(std::move(child));
            }
            auto newBody = std::make_unique<BlockStatementNode>(std::move(newProgramNode), emptySourceCodeLocationSpan);
            auto loopStatement = std::make_unique<LoopStatementNode>(std::move(newBody), emptySourceCodeLocationSpan);
            return loopStatement;
        }
        case NodeKind::Program: {
            auto* programNode = static_cast<ProgramNode*>(node.get());
            std::vector<std::unique_ptr<Node>> newChildren;
            auto children = programNode->takeChildren();
            for (auto& child : children) {
                newChildren.push_back(_desugar(std::move(child)));
            }
            programNode->setChildren(std::move(newChildren));
            return unique_ptr_static_cast<ProgramNode>(std::move(node));
        }
        case NodeKind::VariableDeclaration: {
            auto* variableDeclarationNode = static_cast<VariableDeclarationNode*>(node.get());
            variableDeclarationNode->setIdentifier(unique_ptr_static_cast<IdentifierNode>(_desugar(variableDeclarationNode->takeIdentifier())));
            variableDeclarationNode->setExpression(unique_ptr_static_cast<ExpressionNode>(_desugar(variableDeclarationNode->takeExpression())));
            // TODO handle type annotation at some point
            return unique_ptr_static_cast<VariableDeclarationNode>(std::move(node));
        }
        case NodeKind::AssignmentExpression: {
            auto* assignmentExpressionNode = static_cast<AssignmentExpressionNode*>(node.get());
            std::unique_ptr<IdentifierNode> identifierNode = unique_ptr_static_cast<IdentifierNode>(_desugar(assignmentExpressionNode->takeIdentifier()));
            std::unique_ptr<ExpressionNode> expressionNode = unique_ptr_static_cast<ExpressionNode>(_desugar(assignmentExpressionNode->takeExpression()));
            assignmentExpressionNode->setIdentifier(std::move(identifierNode));
            assignmentExpressionNode->setExpression(std::move(expressionNode));
            return unique_ptr_static_cast<AssignmentExpressionNode>(std::move(node));
        }
        case NodeKind::BlockStatement: {
            auto* blockStatementNode = static_cast<BlockStatementNode*>(node.get());
            blockStatementNode->setProgramNode(unique_ptr_static_cast<ProgramNode>(_desugar(blockStatementNode->takeProgramNode())));
            return unique_ptr_static_cast<BlockStatementNode>(std::move(node));
        }
        case NodeKind::FunctionDeclaration: {
            auto* functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node.get());
            functionDeclarationNode->setIdentifier(unique_ptr_static_cast<IdentifierNode>(_desugar(functionDeclarationNode->takeIdentifier())));
            std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> newParameters;
            for (auto& parameter : functionDeclarationNode->takeParameters()) {
                newParameters.push_back(unique_ptr_static_cast<IdentifierWithPossibleAnnotationNode>(_desugar(std::move(parameter))));
            }
            functionDeclarationNode->setParameters(std::move(newParameters));
            functionDeclarationNode->setBodyNode(unique_ptr_static_cast<BlockStatementNode>(_desugar(functionDeclarationNode->takeBodyNode())));
            return unique_ptr_static_cast<FunctionDeclarationNode>(std::move(node));
        }
        case NodeKind::IfStatement: {
            auto* ifStatementNode = static_cast<IfStatementNode*>(node.get());
            ifStatementNode->setCondition(unique_ptr_static_cast<ExpressionNode>(_desugar(ifStatementNode->takeCondition())));
            ifStatementNode->setThenBranch(_desugar(ifStatementNode->takeThenBranch()));
            if (ifStatementNode->getElseBranch()) {
                ifStatementNode->setElseBranch(_desugar(ifStatementNode->takeElseBranch()));
            }
            return unique_ptr_static_cast<IfStatementNode>(std::move(node));
        }
        case NodeKind::LoopStatement: {
            auto* loopStatementNode = static_cast<LoopStatementNode*>(node.get());
            loopStatementNode->setBody(unique_ptr_static_cast<BlockStatementNode>(_desugar(loopStatementNode->takeBody())));
            return unique_ptr_static_cast<LoopStatementNode>(std::move(node));
        }
        case NodeKind::ReturnStatement: {
            auto* returnStatementNode = static_cast<ReturnStatementNode*>(node.get());
            if (returnStatementNode->getExpression()) {
                returnStatementNode->setExpression(unique_ptr_static_cast<ExpressionNode>(_desugar(returnStatementNode->takeExpression())));
            }
            return unique_ptr_static_cast<ReturnStatementNode>(std::move(node));
        }
        case NodeKind::AssignmentStatement: {
            auto* assignmentStatementNode = static_cast<AssignmentStatementNode*>(node.get());
            assignmentStatementNode->setAssignmentExpression(unique_ptr_static_cast<AssignmentExpressionNode>(_desugar(assignmentStatementNode->takeAssignmentExpression())));
            return unique_ptr_static_cast<AssignmentStatementNode>(std::move(node));
        }
        case NodeKind::FunctionCallExpression: {
            auto* functionCallExpressionNode = static_cast<FunctionCallExpressionNode*>(node.get());
            functionCallExpressionNode->setIdentifier(unique_ptr_static_cast<IdentifierNode>(_desugar(functionCallExpressionNode->takeIdentifier())));
            std::vector<std::unique_ptr<ExpressionNode>> newArgumentNodes;
            for (auto& argument : functionCallExpressionNode->takeArgumentNodes()) {
                newArgumentNodes.push_back(unique_ptr_static_cast<ExpressionNode>(_desugar(std::move(argument))));
            }
            functionCallExpressionNode->setArgumentNodes(std::move(newArgumentNodes));
            return unique_ptr_static_cast<FunctionCallExpressionNode>(std::move(node));
        }
        case NodeKind::FunctionCallStatement: {
            auto* functionCallStatementNode = static_cast<FunctionCallStatementNode*>(node.get());
            functionCallStatementNode->setFunctionCallExpression(unique_ptr_static_cast<FunctionCallExpressionNode>(_desugar(functionCallStatementNode->takeFunctionCallExpression())));
            return unique_ptr_static_cast<FunctionCallStatementNode>(std::move(node));
        }
        case NodeKind::BinaryOperatorExpression: {
            auto* binaryOperatorExpressionNode = static_cast<BinaryOperatorExpressionNode*>(node.get());
            binaryOperatorExpressionNode->setLeft(unique_ptr_static_cast<ExpressionNode>(_desugar(binaryOperatorExpressionNode->takeLeft())));
            binaryOperatorExpressionNode->setRight(unique_ptr_static_cast<ExpressionNode>(_desugar(binaryOperatorExpressionNode->takeRight())));
            return unique_ptr_static_cast<BinaryOperatorExpressionNode>(std::move(node));
        }
        case NodeKind::UnaryOperatorExpression: {
            auto* unaryOperatorExpressionNode = static_cast<UnaryOperatorExpressionNode*>(node.get());
            unaryOperatorExpressionNode->setOperand(unique_ptr_static_cast<ExpressionNode>(_desugar(unaryOperatorExpressionNode->takeOperand())));
            return unique_ptr_static_cast<UnaryOperatorExpressionNode>(std::move(node));
        }
        case NodeKind::IfExpression: {
            auto* ifExpressionNode = static_cast<IfExpressionNode*>(node.get());
            ifExpressionNode->setCondition(unique_ptr_static_cast<ExpressionNode>(_desugar(ifExpressionNode->takeCondition())));
            ifExpressionNode->setThenBranch(unique_ptr_static_cast<ExpressionNode>(_desugar(ifExpressionNode->takeThenBranch())));
            if (ifExpressionNode->getElseBranch()) {
                ifExpressionNode->setElseBranch(unique_ptr_static_cast<ExpressionNode>(_desugar(ifExpressionNode->takeElseBranch())));
            }
            return unique_ptr_static_cast<IfExpressionNode>(std::move(node));
        }
        case NodeKind::Identifier:
        case NodeKind::IdentifierWithPossibleAnnotation:
        case NodeKind::TypeExpression:
        case NodeKind::NumberLiteral:
        case NodeKind::StringLiteral:
        case NodeKind::BooleanLiteral:
        case NodeKind::EmptyLiteral:
        case NodeKind::BreakStatement:
        case NodeKind::ContinueStatement:
        case NodeKind::Invalid:
            return node;
    }
}

std::unique_ptr<Node> Desugarer::desugar() {
    this->root = _desugar(std::move(this->root));
    return std::move(this->root);
}