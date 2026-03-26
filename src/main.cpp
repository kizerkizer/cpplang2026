#include <print>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "lexer/lexer.hpp"
#include "parser/node.hpp"
#include "parser/parser.hpp"
#include "lexer/token.hpp"
#include "desugarer/desugarer.hpp"
#include "binder/binder.hpp"
#include "binder/name.hpp"

void printParseTree (const Node* node, int indentation) {
    switch (node->getNodeKind()) {
        case NodeKind::Program: {
            const auto nodeCast = static_cast<const ProgramNode*>(node);
            std::print("{}ProgramNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Children:\n", std::string(indentation * 2, ' '));
            for (const auto child : nodeCast->getChildren()) {
                printParseTree(child, indentation + 1);
            }
            break;
        }
        case NodeKind::VariableDeclaration: {
            const auto nodeCast = static_cast<const VariableDeclarationNode*>(node);
            std::print("{}VariableDeclarationNode\n", std::string(indentation * 2, ' '));
            std::print("{}* AssignmentExpression:\n", std::string(indentation * 2, ' '));
            if (nodeCast->getAssignmentExpression()) {
                printParseTree(nodeCast->getAssignmentExpression(), indentation + 1);
            } else {
                std::print("{}None\n", std::string((indentation + 1) * 2, ' '));
            }
            break;
        }
        case NodeKind::FunctionDeclaration: {
            const auto nodeCast = static_cast<const FunctionDeclarationNode*>(node);
            std::print("{}FunctionDeclarationNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Identifier: {}\n", std::string(indentation * 2, ' '), nodeCast->getIdentifier()->getName());
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
            if (nodeCast->getNameReference()) {
                std::print("{}☝️ NameReference:\n", std::string(indentation * 2, ' '));
                auto name = nodeCast->getNameReference();
                std::print("{}'{}'\n", std::string((indentation + 1) * 2, ' '), name->getNameString());
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
        case NodeKind::NumberLiteral: {
            const auto nodeCast = static_cast<const NumberLiteralNode*>(node);
            std::print("{}NumberLiteralNode\n", std::string(indentation * 2, ' '));
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
            std::print("{}* ProgramNode:\n", std::string(indentation * 2, ' '));
            printParseTree(nodeCast->getProgramNode(), indentation + 1);
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
    }
}

void printScope (Scope* scope, int indentation) {
    std::print("{}Scope ({})\n", std::string(indentation * 2, ' '), scopeKindToString(scope->getKind()));
    std::print("{}* Names:\n", std::string(indentation * 2, ' '));
    for (const auto [nameString, name] : scope->getNames()) {
        std::print("{}- {} ({})\n", std::string((indentation + 1) * 2, ' '), nameString, nameKindToString(name->getKind()));
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

int main () {
    std::print("Lexing...\n");
    auto filename = "/Users/alex/Documents/Projects/cpp-lang/sampleFiles/source.txt";
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::print("Failed to open file: {}\n", filename);
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sourceString = buffer.str();
    Lexer lexer;
    std::vector<std::string> errorMessages;
    auto tokens = lexer.lex(sourceString, errorMessages);
    if (errorMessages.empty()) {
        std::print("No lex errors.\n");
    } else {
        std::print("⚠️ {} lex error(s) found.\n", errorMessages.size());
    }
    for (const auto& token : tokens) {
        std::print("{}\n", token.toString());
    }
    for (const auto& errorMessage : errorMessages) {
        std::print("{}\n", errorMessage);
    }
    if (!errorMessages.empty()) {
        return 1;
    }
    std::vector<Token> nonTrivialTokens;
    std::copy_if(tokens.begin(), tokens.end(), std::back_inserter(nonTrivialTokens), [](const Token& token) {
        return !IS_TOKENNAME_TRIVIA(token.getTokenName());
    });
    Parser parser(nonTrivialTokens, errorMessages);
    auto parsed = parser.parse();
    if (errorMessages.empty()) {
        std::print("No parse errors.\n");
    } else {
        std::print("⚠️ {} parse error(s) found.\n", errorMessages.size());
    }
    for (const auto& errorMessage : errorMessages) {
        std::print("{}\n", errorMessage);
    }
    if (!errorMessages.empty()) {
        return 1;
    }
    auto desugared = Desugarer(std::move(parsed)).desugar();
    //std::print("Desugared parse tree:\n");
    //printParseTree(desugared.get(), 0);
    Binder binder(errorMessages);
    auto binderResult = binder.bind(std::move(desugared));
    if (errorMessages.empty()) {
        std::print("No binder errors.\n");
    } else {
        std::print("⚠️ {} binder error(s) found.\n", errorMessages.size());
    }
    for (const auto& errorMessage : errorMessages) {
        std::print("{}\n", errorMessage);
    }
    if (!errorMessages.empty()) {
        return 1;
    }
    printBinderResult(std::move(binderResult));
    return 0;
}