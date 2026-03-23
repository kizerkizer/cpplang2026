#include <print>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "lexer.hpp"
#include "node.hpp"
#include "parser.hpp"
#include "token.hpp"

void printParseTree (const Node& node, int indentation) {
    switch (node.getNodeType()) {
        case NodeType::Program: {
            const auto& nodeCast = static_cast<const ProgramNode&>(node);
            std::print("{}ProgramNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Children:\n", std::string(indentation * 2, ' '));
            for (const auto& child : nodeCast.getChildren()) {
                printParseTree(*child.get(), indentation + 1);
            }
            break;
        }
        case NodeType::VariableDeclaration: {
            const auto& nodeCast = static_cast<const VariableDeclarationNode&>(node);
            std::print("{}VariableDeclarationNode\n", std::string(indentation * 2, ' '));
            std::print("{}* AssignmentExpression:\n", std::string(indentation * 2, ' '));
            if (nodeCast.getAssignmentExpression()) {
                printParseTree(*nodeCast.getAssignmentExpression()->getExpression(), indentation + 1);
            } else {
                std::print("{}None\n", std::string((indentation + 1) * 2, ' '));
            }
            break;
        }
        case NodeType::FunctionDeclaration: {
            const auto& nodeCast = static_cast<const FunctionDeclarationNode&>(node);
            std::print("{}FunctionDeclarationNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Identifier: {}\n", std::string(indentation * 2, ' '), nodeCast.getIdentifier()->getName());
            std::print("{}* Parameters:\n", std::string(indentation * 2, ' '));
            for (const auto& param : nodeCast.getParameters()) {
                printParseTree(*param.get(), indentation + 1);
            }
            std::print("{}* Body:\n", std::string(indentation * 2, ' '));
            printParseTree(*nodeCast.getBody(), indentation + 1);
            break;
        }
        case NodeType::AssignmentExpression: {
            const auto& nodeCast = static_cast<const AssignmentExpressionNode&>(node);
            std::print("{}AssignmentExpressionNode:\n", std::string(indentation * 2, ' '));
            std::print("{}* Identifier:\n", std::string(indentation * 2, ' '));
            printParseTree(*nodeCast.getIdentifier(), indentation + 1);
            std::print("{}* Expression:\n", std::string(indentation * 2, ' '));
            printParseTree(*nodeCast.getExpression(), indentation + 1);
            break;
        }
        case NodeType::FunctionCallExpression: {
            const auto& nodeCast = static_cast<const FunctionCallExpressionNode&>(node);
            std::print("{}FunctionCallExpressionNode: {}\n", std::string(indentation * 2, ' '), nodeCast.getIdentifier()->getName());
            std::print("{}* Identifier:\n", std::string(indentation * 2, ' '));
            printParseTree(*nodeCast.getIdentifier(), indentation + 1);
            std::print("{}* Arguments:\n", std::string(indentation * 2, ' '));
            for (const auto& argumentNode : nodeCast.getArgumentNodes()) {
                printParseTree(*argumentNode.get(), indentation + 1);
            }
            break;
        }
        case NodeType::Identifier: {
            const auto& nodeCast = static_cast<const IdentifierNode&>(node);
            std::print("{}IdentifierNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Name:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast.getName());
            break;
        }
        case NodeType::Invalid:
            std::print("{}InvalidNode\n", std::string(indentation * 2, ' '));
            break;
        case NodeType::IfStatement: {
            const auto& nodeCast = static_cast<const IfStatementNode&>(node);
            std::print("{}IfStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Condition:\n", std::string(indentation * 2, ' '));
            printParseTree(*nodeCast.getCondition(), indentation + 1);
            std::print("{}* ThenBranch:\n", std::string(indentation * 2, ' '));
            printParseTree(*nodeCast.getThenBranch(), indentation + 1);
            std::print("{}* ElseBranch:\n", std::string(indentation * 2, ' '));
            if (nodeCast.getElseBranch()) {
                printParseTree(*nodeCast.getElseBranch(), indentation + 1);
            } else {
                std::print("{}None\n", std::string((indentation + 1) * 2, ' '));
            }
            break;
        }
        case NodeType::WhileStatement: {
            const auto& nodeCast = static_cast<const WhileStatementNode&>(node);
            std::print("{}WhileStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Condition:\n", std::string(indentation * 2, ' '));
            printParseTree(*nodeCast.getCondition(), indentation + 1);
            std::print("{}* Body:\n", std::string(indentation * 2, ' '));
            printParseTree(*nodeCast.getBody(), indentation + 1);
            break;
        }
        case NodeType::ReturnStatement: {
            const auto& nodeCast = static_cast<const ReturnStatementNode&>(node);
            std::print("{}ReturnStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Expression:\n", std::string(indentation * 2, ' '));
            if (nodeCast.getExpression()) {
                printParseTree(*nodeCast.getExpression(), indentation + 1);
            } else {
                std::print("{}None\n", std::string((indentation + 1) * 2, ' '));
            }
            break;
        }
        case NodeType::BreakStatement:
            std::print("{}BreakStatementNode\n", std::string(indentation * 2, ' '));
            break;
        case NodeType::ContinueStatement:
            std::print("{}ContinueStatementNode\n", std::string(indentation * 2, ' '));
            break;
        case NodeType::NumberLiteral: {
            const auto& nodeCast = static_cast<const NumberLiteralNode&>(node);
            std::print("{}NumberLiteralNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Value:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast.getValue());
            break;
        }
        case NodeType::StringLiteral: {
            const auto& nodeCast = static_cast<const StringLiteralNode&>(node);
            std::print("{}StringLiteralNode\n", std::string(indentation * 2, ' '));
            std::print("{}* Value:\n", std::string(indentation * 2, ' '));
            std::print("{}{}\n", std::string((indentation + 1) * 2, ' '), nodeCast.getValue());
            break;
        }
        case NodeType::BlockStatement: {
            const auto& nodeCast = static_cast<const BlockStatementNode&>(node);
            std::print("{}BlockStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* ProgramNode:\n", std::string(indentation * 2, ' '));
            printParseTree(*nodeCast.getProgramNode(), indentation + 1);
            break;
        }
        case NodeType::AssignmentStatement: {
            const auto& nodeCast = static_cast<const AssignmentStatementNode&>(node);
            std::print("{}AssignmentStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* AssignmentExpression:\n", std::string(indentation * 2, ' '));
            printParseTree(*nodeCast.getAssignmentExpression(), indentation + 1);
            break;
        }
        case NodeType::FunctionCallStatement: {
            const auto& nodeCast = static_cast<const FunctionCallStatementNode&>(node);
            std::print("{}FunctionCallStatementNode\n", std::string(indentation * 2, ' '));
            std::print("{}* FunctionCallExpression:\n", std::string(indentation * 2, ' '));
            printParseTree(*nodeCast.getFunctionCallExpression(), indentation + 1);
            break;
        }
        default:
            std::print("{}Unknown node type\n", std::string(indentation * 2, ' '));
            break;
    }
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
    }
    for (const auto& errorMessage : errorMessages) {
        std::print("{}\n", errorMessage);
    }
    printParseTree(*parsed, 0);
    if (!errorMessages.empty()) {
        return 1;
    }
    return 0;
}
