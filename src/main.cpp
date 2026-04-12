#include <memory>
#include <print>
#include <fstream>
#include <string>
#include <sstream>
#include <set>

#include "common/sourcecodelocation.hpp"
#include "flowbuilder/flowbuilder.hpp"
#include "flowbuilder/flownode.hpp"
#include "parser/node.hpp"
#include "lexer/token.hpp"
#include "binder/binder.hpp"
#include "binder/symbol.hpp"
#include "checker/type.hpp"
#include "treewalker/outputstream.hpp"
#include "driver/driver.hpp"

void printParseTree (const Node* node, int indentation) {
    auto sourceCodeLocationSpan = node->getSourceCodeLocationSpan();
    std::print("{}{}\n", std::string(indentation * 2, ' '), englishbreakfast::sourceCodeLocationSpanToString(sourceCodeLocationSpan));
    switch (node->getNodeKind()) {
        case NodeKind::Program: {
            const auto nodeCast = static_cast<const ProgramNode*>(node);
            std::print("{}ProgramNode\n", std::string(indentation * 2, ' '));
            std::print("{}* ExecutionListNode:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getExecutionListNode(), indentation + 1);
            break;
        }
        case NodeKind::VariableDeclaration: {
            const auto nodeCast = static_cast<const VariableDeclarationNode*>(node);
            std::print("{}VariableDeclarationNode\n", std::string(indentation * 2, ' '));
            std::print("{}* IdentifierNode\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getIdentifier(), indentation + 1);
            std::print("{}* TypeAnnotationNode\n", std::string(indentation * 2, ' '));
            if (nodeCast->getTypeAnnotation()) {
                printParseTree(nodeCast->getTypeAnnotation(), indentation + 1);
            } else {
                std::print("{}None\n", std::string((indentation + 1) * 2, ' '));
            }
            std::print("{}* ExpressionNode\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getExpression(), indentation + 1);
            break;
        }
        case NodeKind::FunctionDeclaration: {
            const auto nodeCast = static_cast<const FunctionDeclarationNode*>(node);
            std::print("{}FunctionDeclarationNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Identifier: {}\n", std::string(indentation * 2, ' '), nodeCast->getIdentifier()->getName());
            std::print("{}* ReturnTypeExpression:\n", std::string(indentation * 2, ' '));
            if (nodeCast->getReturnTypeAnnotation()) {
                printParseTree(nodeCast->getReturnTypeAnnotation(), indentation + 1);
            } else {
                std::print("{}None\n", std::string((indentation + 1) * 2, ' '));
            }
            std::print("{}* Parameters:\n", std::string(indentation * 2, ' '));
            for (const auto param : nodeCast->getParameters()) {
                printParseTree(param, indentation + 1);
            }
            std::print("{}* Body:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getBody(), indentation + 1);
            break;
        }
        case NodeKind::AssignmentExpression: {
            const auto nodeCast = static_cast<const AssignmentExpressionNode*>(node);
            std::print("{}AssignmentExpressionNode:\n", std::string(indentation * 2, ' '));
            std::print("{}* Identifier:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getIdentifier(), indentation + 1);
            std::print("{}* Expression:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getExpression(), indentation + 1);
            break;
        }
        case NodeKind::FunctionCallExpression: {
            const auto nodeCast = static_cast<const FunctionCallExpressionNode*>(node);
            std::print("{}FunctionCallExpressionNode: {}\n", std::string(indentation * 2, ' '), nodeCast->getIdentifier()->getName());
            std::print("{}* Identifier:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getIdentifier(), indentation + 1);
            std::print("{}* Arguments:\n", std::string(indentation * 2, ' '));
            for (const auto argumentNode : nodeCast->getArgumentNodes()) {
                printParseTree(argumentNode, indentation + 1);
            }
            break;
        }
        case NodeKind::Identifier: {
            const auto nodeCast = static_cast<const IdentifierNode*>(node);
            std::print("{}IdentifierNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Name:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast->getName());
            break;
        }
        case NodeKind::IdentifierWithPossibleAnnotation: {
            const auto nodeCast = static_cast<const IdentifierWithPossibleAnnotationNode*>(node);
            std::print("{}IdentifierWithPossibleAnnotationNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Name:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast->getName());
            std::print("{}* Annotation:\n", std::string(indentation * 2, ' '));
            auto annotation = nodeCast->getAnnotation();
            if (annotation) {
                printParseTree(annotation, indentation + 1);
            } else {
                std::print("{}None\n", std::string((indentation + 1) * 2, ' '));
            }
            break;
        }
        case NodeKind::Invalid:
            std::print("{}InvalidNode\n", std::string(indentation * 2, ' '));
            break;
        case NodeKind::IfStatement: {
            const auto nodeCast = static_cast<const IfStatementNode*>(node);
            std::print("{}IfStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Condition:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getCondition(), indentation + 1);
            std::print("{}* ThenBranch:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getThenBranch(), indentation + 1);
            std::print("{}* ElseBranch:\n", std::string(indentation * 2, ' '));
            if (nodeCast->getElseBranch()) {
                printParseTree(nodeCast->getElseBranch(), indentation + 1);
            } else {
                std::print("{}None\n", std::string((indentation + 1) * 2, ' '));
            }
            break;
        }
        case NodeKind::WhileStatement: {
            const auto nodeCast = static_cast<const WhileStatementNode*>(node);
            std::print("{}WhileStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Condition:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getCondition(), indentation + 1);
            std::print("{}* Body:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getBody(), indentation + 1);
            break;
        }
        case NodeKind::LoopStatement: {
            const auto nodeCast = static_cast<const LoopStatementNode*>(node);
            std::print("{}LoopStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Body:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getBody(), indentation + 1);
            break;
        }
        case NodeKind::ReturnStatement: {
            const auto nodeCast = static_cast<const ReturnStatementNode*>(node);
            std::print("{}ReturnStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Expression:\n", std::string(indentation * 2, ' '));
            if (nodeCast->getExpression()) {
                printParseTree(nodeCast->getExpression(), indentation + 1);
            } else {
                std::print("{}None\n", std::string((indentation + 1) * 2, ' '));
            }
            break;
        }
        case NodeKind::BreakStatement:
            std::print("{}BreakStatementNode\n", std::string(indentation * 2, ' '));
            break;
        case NodeKind::ContinueStatement:
            std::print("{}ContinueStatementNode\n", std::string(indentation * 2, ' '));
            break;
        case NodeKind::EmptyLiteral: {
            std::print("{}EmptyLiteralNode\n", std::string(indentation * 2, ' '));
            break;
        }
        case NodeKind::IntegerLiteral: {
            const auto nodeCast = static_cast<const IntegerLiteralNode*>(node);
            std::print("{}IntegerLiteralNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Value:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast->getValue());
            break;
        }
        case NodeKind::FloatLiteral: {
            const auto nodeCast = static_cast<const FloatLiteralNode*>(node);
            std::print("{}FloatLiteralNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Value:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast->getValue());
            break;
        }
        case NodeKind::StringLiteral: {
            const auto nodeCast = static_cast<const StringLiteralNode*>(node);
            std::print("{}StringLiteralNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Value:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast->getValue());
            break;
        }
        case NodeKind::BlockStatement: {
            const auto nodeCast = static_cast<const BlockStatementNode*>(node);
            std::print("{}BlockStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* ExecutionListNode:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getExecutionListNode(), indentation + 1);
            break;
        }
        case NodeKind::ExecutionList: {
            const auto nodeCast = static_cast<const ExecutionListNode*>(node);
            std::print("{}ExecutionListNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Children:\n", std::string(indentation * 2, ' '));
            for (const auto child : nodeCast->getChildren()) {
                printParseTree(child, indentation + 1);
            }
            break;
        }
        case NodeKind::AssignmentStatement: {
            const auto nodeCast = static_cast<const AssignmentStatementNode*>(node);
            std::print("{}AssignmentStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* AssignmentExpression:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getAssignmentExpression(), indentation + 1);
            break;
        }
        case NodeKind::FunctionCallStatement: {
            const auto nodeCast = static_cast<const FunctionCallStatementNode*>(node);
            std::print("{}FunctionCallStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* FunctionCallExpression:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getFunctionCallExpression(), indentation + 1);
            break;
        }
        case NodeKind::BinaryOperatorExpression: {
            const auto nodeCast = static_cast<const BinaryOperatorExpressionNode*>(node);
            std::print("{}BinaryOperatorExpressionNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Operator:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast->getOperatorToken()->getSourceString());
            std::print("{}* Left:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getLeft(), indentation + 1);
            std::print("{}* Right:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getRight(), indentation + 1);
            break;
        }
        case NodeKind::UnaryOperatorExpression: {
            const auto nodeCast = static_cast<const UnaryOperatorExpressionNode*>(node);
            std::print("{}UnaryOperatorExpressionNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Operator:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast->getOperatorToken()->getSourceString());
            std::print("{}* Expression:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getOperand(), indentation + 1);
            break;
        }
        case NodeKind::IfExpression: {
            const auto nodeCast = static_cast<const IfExpressionNode*>(node);
            std::print("{}IfExpressionNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Condition:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getCondition(), indentation + 1);
            std::print("{}* Then Branch:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getThenBranch(), indentation + 1);
            std::print("{}* Else Branch:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getElseBranch(), indentation + 1);
            break;
        }
        case NodeKind::BooleanLiteral: {
            const auto nodeCast = static_cast<const BooleanLiteralNode*>(node);
            std::print("{}BooleanLiteralNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Value:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast->getValue() ? "true" : "false");
            break;
        }
        case NodeKind::TypeDeclaration: {
            const auto nodeCast = static_cast<const TypeDeclarationNode*>(node);
            std::print("{}TypeDeclarationNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Identifier:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getIdentifier(), indentation + 1);
            std::print("{}* TypeExpression:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getTypeExpression(), indentation + 1);
            break;
        }
        case NodeKind::TypeIdentifier: {
            const auto nodeCast = static_cast<const TypeIdentifierNode*>(node);
            std::print("{}TypeIdentifierNode\n", std::string(indentation * 2, ' '));
            std::print("{}* IdentifierToken:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast->getIdentifierToken()->getSourceString());
            break;
        }
        case NodeKind::TypePrimitive: {
            const auto nodeCast = static_cast<const TypePrimitiveNode*>(node);
            std::print("{}TypePrimitiveNode\n", std::string(indentation * 2, ' '));
            std::print("{}* PrimitiveTypeKind:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), primitiveTypeToString(nodeCast->getPrimitiveTypeKind()));
            break;
        }
        case NodeKind::BinaryOperatorTypeExpression: {
            const auto nodeCast = static_cast<const BinaryOperatorTypeExpressionNode*>(node);
            std::print("{}BinaryOperatorTypeExpressionNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Operator:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast->getOperatorToken()->getSourceString());
            std::print("{}* Left:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getLeft(), indentation + 1);
            std::print("{}* Right:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getRight(), indentation + 1);
            break;
        }
    }
}

void printScope (Scope* scope, int indentation) {
    std::print("{}Scope ({})\n", std::string(indentation * 2, ' '), scopeKindToString(scope->getKind()));
    std::print("{}* Names:\n", std::string(indentation * 2, ' '));
    for (const auto [nameString, name] : scope->getSymbols()) {
        std::print("{}- {} ({})\n", std::string((indentation + 1) * 2, ' '), nameString, symbolKindToString(name->getKind()));
    }
    std::print("{}* Child Scopes:\n", std::string(indentation * 2, ' '));
    for (const auto childScope : scope->getChildren()) {
        printScope(childScope, indentation + 1);
    }
}

void printBinderResult (std::unique_ptr<BinderResult> binderResult) {
    std::print("BinderResult:\n");
    auto rootNode = binderResult->getNode();
    auto rootScope = binderResult->getRootScope();
    printScope(rootScope, 0);
    printParseTree(rootNode, 0);
}

std::string getNodeSyntax(Node* node, int indentation) {
    if (node == nullptr) {
        return "";
    }
    switch (node->getNodeKind()) {
        case NodeKind::Program: {
            auto programNode = static_cast<ProgramNode*>(node);
            return getNodeSyntax(programNode->getExecutionListNode(), indentation);
        }
        case NodeKind::ExecutionList: {
            std::string ret = "";
            for (auto child : node->getChildren()) {
                ret += getNodeSyntax(child, indentation);
            }
            return ret;
        }
        case NodeKind::VariableDeclaration: {
            auto variableDeclarationNode = static_cast<VariableDeclarationNode*>(node);
            std::string ret = std::string(indentation * 2, ' ') + "var ";
            ret += variableDeclarationNode->getIdentifier()->getName();
            if (variableDeclarationNode->getTypeAnnotation()) {
                ret += ": ";
                ret += getNodeSyntax(variableDeclarationNode->getTypeAnnotation(), 0);
            }
            if (variableDeclarationNode->getExpression()) {
                ret += " = ";
                ret += getNodeSyntax(variableDeclarationNode->getExpression(), 0);
                ret += ";\n";
            } else {
                ret += ";\n";
            }
            return ret;
        }
        case NodeKind::BlockStatement: {
            auto blockStatementNode = static_cast<BlockStatementNode*>(node);
            std::string ret = std::string(indentation * 2, ' ') + "{\n";
            ret += getNodeSyntax(blockStatementNode->getExecutionListNode(), indentation + 1);
            ret += std::string(indentation * 2, ' ') + "}\n";
            return ret;
        }
        case NodeKind::FunctionDeclaration: {
            auto functionDeclarationNode = static_cast<FunctionDeclarationNode*>(node);
            std::string ret = std::string(indentation * 2, ' ') + "function ";
            ret += functionDeclarationNode->getIdentifierName();
            ret += " (";
            auto parameters = functionDeclarationNode->getParameters();
            for (auto parameter : parameters) {
                ret += parameter->getName();
                if (parameter->getAnnotation()) {
                    ret += ": " + getNodeSyntax(parameter->getAnnotation(), 0);
                }
                if (parameters.back() != parameter) {
                    ret += ", ";
                }
            }
            ret += ")";
            if (functionDeclarationNode->getReturnTypeAnnotation()) {
                ret += ": ";
                ret += getNodeSyntax(functionDeclarationNode->getReturnTypeAnnotation(), 0);
            }
            auto body = functionDeclarationNode->getBody();
            ret += " ";
            ret += getNodeSyntax(body, indentation);
            ret += "\n";
            return ret;
        }
        case NodeKind::TypeDeclaration: {
            auto typeDeclarationNode = static_cast<TypeDeclarationNode*>(node);
            std::string ret = std::string(indentation * 2, ' ') + "type ";
            ret += typeDeclarationNode->getIdentifier()->getName();
            ret += " = ";
            ret += getNodeSyntax(typeDeclarationNode->getTypeExpression(), 0);
            ret += ";\n";
            return ret;
        }
        case NodeKind::TypeIdentifier: {
            auto typeIdentifierNode = static_cast<TypeIdentifierNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += typeIdentifierNode->getIdentifierToken()->getSourceString();
            return ret;
        }
        case NodeKind::TypePrimitive: {
            auto typePrimitiveNode = static_cast<TypePrimitiveNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += primitiveTypeToString(typePrimitiveNode->getPrimitiveTypeKind());
            return ret;
        }
        case NodeKind::BinaryOperatorTypeExpression: {
            auto binaryOperatorTypeExpressionNode = static_cast<BinaryOperatorTypeExpressionNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += getNodeSyntax(binaryOperatorTypeExpressionNode->getLeft(), 0);
            ret += " ";
            ret += binaryOperatorTypeExpressionNode->getOperatorToken()->getSourceString();
            ret += " ";
            ret += getNodeSyntax(binaryOperatorTypeExpressionNode->getRight(), 0);
            return ret;
        }
        case NodeKind::IdentifierWithPossibleAnnotation: {
            auto identifierWithPossibleAnnotationNode = static_cast<IdentifierWithPossibleAnnotationNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += identifierWithPossibleAnnotationNode->getIdentifierToken()->getSourceString();
            if (identifierWithPossibleAnnotationNode->getAnnotation()) {
                ret += ": ";
                ret += getNodeSyntax(identifierWithPossibleAnnotationNode->getAnnotation(), 0);
            }
            return ret;
        }
        case NodeKind::Identifier: {
            auto identifierNode = static_cast<IdentifierNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += identifierNode->getName();
            return ret;
        }
        case NodeKind::AssignmentExpression: {
            auto assignmentExpressionNode = static_cast<AssignmentExpressionNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += getNodeSyntax(assignmentExpressionNode->getIdentifier(), 0);
            ret += " = ";
            ret += getNodeSyntax(assignmentExpressionNode->getExpression(), 0);
            return ret;
        }
        case NodeKind::ReturnStatement: {
            auto returnStatementNode = static_cast<ReturnStatementNode*>(node);
            std::string ret = std::string(indentation * 2, ' ') + "return";
            if (returnStatementNode->getExpression()) {
                ret += " " + getNodeSyntax(returnStatementNode->getExpression(), 0);
            }
            ret += ";\n";
            return ret;
        }
        case NodeKind::AssignmentStatement: {
            auto assignmentStatementNode = static_cast<AssignmentStatementNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += getNodeSyntax(assignmentStatementNode->getAssignmentExpression(), 0);
            ret += ";\n";
            return ret;
        }
        case NodeKind::BinaryOperatorExpression: {
            auto binaryOperatorExpressionNode = static_cast<BinaryOperatorExpressionNode*>(node);
            auto ret = getNodeSyntax(binaryOperatorExpressionNode->getLeft(), indentation);
            ret += " ";
            ret += binaryOperatorExpressionNode->getOperatorToken()->getSourceString();
            ret += " ";
            ret += getNodeSyntax(binaryOperatorExpressionNode->getRight(), 0);
            return ret;
        }
        case NodeKind::FunctionCallExpression: {
            auto functionCallExpressionNode = static_cast<FunctionCallExpressionNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += functionCallExpressionNode->getIdentifier()->getName();
            ret += "(";
            auto arguments = functionCallExpressionNode->getArgumentNodes();
            for (auto argument : arguments) {
                ret += getNodeSyntax(argument, 0);
                if (arguments.back() != argument) {
                    ret += ", ";
                }
            }
            ret += ")";
            return ret;
        }
        case NodeKind::FunctionCallStatement: {
            auto functionCallStatementNode = static_cast<FunctionCallStatementNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += getNodeSyntax(functionCallStatementNode->getFunctionCallExpression(), indentation);
            ret += ";\n";
            return ret;
        }
        case NodeKind::BreakStatement: {
            return std::string(indentation * 2, ' ') + "break;\n";
        }
        case NodeKind::ContinueStatement: {
            return std::string(indentation * 2, ' ') + "continue;\n";
        }
        case NodeKind::StringLiteral: {
            auto stringLiteralNode = static_cast<StringLiteralNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += stringLiteralNode->getValue(); // double check; should include quotes
            return ret;
        }
        case NodeKind::IntegerLiteral: {
            auto integerLiteralNode = static_cast<IntegerLiteralNode*>(node);
            return std::string(indentation * 2, ' ') + std::string(integerLiteralNode->getIntegerLiteralToken()->getSourceString());
        }
        case NodeKind::FloatLiteral: {
            auto floatLiteralNode = static_cast<FloatLiteralNode*>(node);
            return std::string(indentation * 2, ' ') + std::string(floatLiteralNode->getFloatLiteralToken()->getSourceString());
        }
        case NodeKind::BooleanLiteral: {
            auto booleanLiteralNode = static_cast<BooleanLiteralNode*>(node);
            return std::string(indentation * 2, ' ') + std::string(booleanLiteralNode->getBooleanLiteralToken()->getSourceString());
        }
        case NodeKind::EmptyLiteral: {
            return std::string(indentation * 2, ' ') + "empty";
        }
        case NodeKind::IfExpression: {
            auto ifExpressionNode = static_cast<IfExpressionNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += "if (";
            ret += getNodeSyntax(ifExpressionNode->getCondition(), 0);
            ret += ") then ";
            ret += getNodeSyntax(ifExpressionNode->getThenBranch(), 0);
            ret += " else ";
            ret += getNodeSyntax(ifExpressionNode->getElseBranch(), 0);
            return ret;
        }
        case NodeKind::IfStatement: {
            auto ifStatementNode = static_cast<IfStatementNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += "if (";
            ret += getNodeSyntax(ifStatementNode->getCondition(), 0);
            ret += ") \n";
            ret += std::string((indentation + 1) * 2, ' ') + "then\n";
            ret += getNodeSyntax(ifStatementNode->getThenBranch(), indentation + 2);
            if (ifStatementNode->getElseBranch()) {
                ret += std::string((indentation + 1) * 2, ' ') + "else\n";
                ret += getNodeSyntax(ifStatementNode->getElseBranch(), indentation + 2);
            }
            return ret;
        }
        case NodeKind::LoopStatement: {
            auto loopStatementNode = static_cast<LoopStatementNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += "loop\n";
            ret += getNodeSyntax(loopStatementNode->getBody(), indentation);
            return ret;
        }
        case NodeKind::WhileStatement: {
            // Shouldn't exist in desugared AST
            return std::string(indentation * 2, ' ') + "<WHILE>"; // TODO
        }
        case NodeKind::Invalid: {
            return std::string(indentation * 2, ' ') + "<INVALID>";
        }
        case NodeKind::UnaryOperatorExpression: {
            auto unaryOperatorExpressionNode = static_cast<UnaryOperatorExpressionNode*>(node);
            std::string ret = std::string(indentation * 2, ' ');
            ret += unaryOperatorExpressionNode->getOperatorToken()->getSourceString();
            ret += "("; // Insert parens just in case
            ret += getNodeSyntax(unaryOperatorExpressionNode->getOperand(), 0);
            ret += ")";
            return ret;
        }
    }
}

std::set<FlowNode*> visited;

void printFlowGraph (FlowGraph* graph);

// Print to DOT language for visualization in Graphviz.
void printFlowNode (FlowNode* flowNode) {
    if (visited.contains(flowNode)) {
        return;
    }
    visited.insert(flowNode);
    for (auto successor : flowNode->getSuccessors()) {
        std::print("n{} -> n{}\n", flowNode->getId(), successor->getId());
    }
    for (auto successor : flowNode->getSuccessors()) {
        printFlowNode(successor);
    }
}

// Print to DOT language for visualization in Graphviz.
void printFlowGraph (FlowGraph* graph) {
    //Node* node = graph->getAstNode();
    //if (node) {
        //std::print("    for {}\n", nodeKindToString(node->getNodeKind()));
    //}
    FlowNode* entry = graph->getEntry();
    printFlowNode(entry);
    visited.clear();
}

void printFlowBuilderResult (std::unique_ptr<FlowBuilderResult> result) {
    int i = 0;
    for (auto graph : result->getGraphs()) {
        std::print("digraph G{} {{\n", i);
        for (auto flowNode : graph->getNodes()) {
            std::string kind = "<None>";
            if (flowNode->getAstNode()) {
                kind = std::string(nodeKindToString(flowNode->getAstNode()->getNodeKind()));
            }
            std::print("n{} [\n", flowNode->getId());
            std::print("    label = \"{}/{}\";\n", flowNodeKindToString(flowNode->getKind()), kind);
            std::print("]\n");
            for (auto successor : flowNode->getSuccessors()) {
                std::print("n{} -> n{}\n", flowNode->getId(), successor->getId());
            }
        }
        i++;
        std::print("}}\n");
    }
}

int main (int argc, char* argv[]) {
    if (argc < 2) {
        std::print("Usage: {} <source-file>\n", argv[0]);
        return 1;
    }
    auto filename = argv[1];
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::print("Failed to open file: {}\n", filename);
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string sourceString = buffer.str();
    std::unique_ptr<Source> source = std::make_unique<Source>(SourceKind::File, filename, sourceString);
    Diagnostics diagnostics = Diagnostics();
    StdOutOutputStream outputStream = StdOutOutputStream();
    Driver driver = Driver(diagnostics, &outputStream);
    driver.compile(source.get());

    return 0;
}