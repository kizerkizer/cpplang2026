#include "parser.hpp"
#include "node.hpp"
#include "token.hpp"

#include <memory>
#include <optional>
#include <string>
#include <print>

Token Parser::peek(int offset) const {
    if (this->index + offset >= this->tokens.size()) {
        return Token("", -1, -1, -1, TokenName::OutOfRange);
    }
    return (this->tokens)[this->index + offset];
}

bool Parser::isPastTokensEnd() const {
    return this->index >= this->tokens.size();
}

std::optional<Token> Parser::expectAndAdvance(const TokenName& expectedTokenName) {
    if (this->isPastTokensEnd()) {
        return std::nullopt;
    }
    Token token = this->peek();
    if (token != expectedTokenName) {
        return std::nullopt;
    }
    this->index++;
    return token;
}

void Parser::enterLoop() {
    this->insideLoop++;
}

void Parser::exitLoop() {
    if (this->insideLoop > 0) {
        this->insideLoop--;
    }
}

void Parser::enterFunction() {
    this->insideFunction++;
}

void Parser::exitFunction() {
    if (this->insideFunction > 0) {
        this->insideFunction--;
    }
}

void Parser::enterBlock() {
    this->insideBlock++;
}

void Parser::exitBlock() {
    if (this->insideBlock > 0) {
        this->insideBlock--;
    }
}

std::unique_ptr<Node> Parser::parse() {
    auto programOpt = this->parseProgram();
    if (!programOpt) {
        return std::make_unique<InvalidNode>();
    }
    return programOpt;
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    std::unique_ptr<ProgramNode> program = std::make_unique<ProgramNode>();
    while (!this->isPastTokensEnd()) {
        auto token = this->peek();
        if (insideBlock && this->peek() == TokenName::BraceClose) {
            this->expectAndAdvance(TokenName::BraceClose);
            return program;
        }
        switch (this->peek().getTokenName()) {
            case TokenName::KeywordVar: {
                auto node = this->parseVariableDeclaration();
                if (!node) {
                    this->errorMessages.push_back("Failed to parse variable declaration at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenName::KeywordFunction: {
                auto node = this->parseFunctionDeclaration();
                if (!node) {
                    this->errorMessages.push_back("Failed to parse function declaration at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenName::KeywordIf: {
                auto node = this->parseIfStatement();
                if (!node) {
                    this->errorMessages.push_back("Failed to parse if statement at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenName::KeywordReturn: {
                if (!this->insideFunction) {
                    this->errorMessages.push_back("Unexpected 'return' statement outside of function at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
                auto node = this->parseReturnStatement();
                if (!node) {
                    this->errorMessages.push_back("Failed to parse return statement at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenName::KeywordBreak: {
                if (!this->insideLoop) {
                    this->errorMessages.push_back("Unexpected 'break' statement outside of while loop at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
                auto node = this->parseBreakStatement();
                if (!node) {
                    this->errorMessages.push_back("Failed to parse break statement at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenName::KeywordContinue: {
                if (!this->insideLoop) {
                    this->errorMessages.push_back("Unexpected 'continue' statement outside of while loop at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
                auto node = this->parseContinueStatement();
                if (!node) {
                    this->errorMessages.push_back("Failed to parse continue statement at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenName::KeywordWhile: {
                auto node = this->parseWhileStatement();
                if (!node) {
                    this->errorMessages.push_back("Failed to parse while statement at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenName::BraceOpen: {
                auto node = this->parseBlockStatement();
                if (!node) {
                    this->errorMessages.push_back("Failed to parse block statement at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenName::Identifier: {
                if (this->peek(1) == TokenName::Equal) {
                    auto node = this->parseAssignmentStatement();
                    if (!node) {
                        this->errorMessages.push_back("Failed to parse variable declaration at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                        return program;
                    }
                    program->addNode(std::move(node));
                    break;
                } else if (this->peek(1) == TokenName::ParenthesisOpen) {
                    auto node = this->parseFunctionCallStatement();
                    if (!node) {
                        this->errorMessages.push_back("Failed to parse function call statement at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                        return program;
                    }
                    program->addNode(std::move(node));
                    break;
                } else {
                    this->errorMessages.push_back("Unexpected token '" + this->peek().getSourceString() + "' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return program;
                }
            }
            default:
                this->errorMessages.push_back("Unexpected token '" + this->peek().getSourceString() + "' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                return program;
        }
    }
    if (this->insideBlock) {
        this->errorMessages.push_back("Expected '}' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return program;
    }
    return program;
};

std::unique_ptr<VariableDeclarationNode> Parser::parseVariableDeclaration() {
    auto varToken = this->expectAndAdvance(TokenName::KeywordVar);
    if (!varToken) {
        this->errorMessages.push_back("Expected 'var' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto identifierToken = this->expectAndAdvance(TokenName::Identifier);
    if (!identifierToken) {
        this->errorMessages.push_back("Expected identifier after 'var' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (this->peek() == TokenName::Semicolon) {
        this->expectAndAdvance(TokenName::Semicolon);
        auto identiferNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value()));
        auto variableDeclaration = std::make_unique<VariableDeclarationNode>(std::move(identiferNode), nullptr);
        return variableDeclaration;
    }
    std::unique_ptr<ExpressionNode> node;
    if (this->expectAndAdvance(TokenName::Equal)) {
        node = this->parseExpression();
    }
    if (!node) {
        this->errorMessages.push_back("Expected expression after variable declaration at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    this->expectAndAdvance(TokenName::Semicolon);
    auto identifierNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value()));
    auto assignmentExpression = std::make_unique<AssignmentExpressionNode>(std::move(identifierNode), std::move(node));
    auto variableDeclaration = std::make_unique<VariableDeclarationNode>(std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value())), std::move(assignmentExpression));
    return variableDeclaration;
}

std::unique_ptr<FunctionDeclarationNode> Parser::parseFunctionDeclaration() {
    auto functionToken = this->expectAndAdvance(TokenName::KeywordFunction);
    if (!functionToken) {
        this->errorMessages.push_back("Expected 'function' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto identifierToken = this->expectAndAdvance(TokenName::Identifier);
    if (!identifierToken) {
        this->errorMessages.push_back("Expected identifier after 'function' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::ParenthesisOpen)) {
        this->errorMessages.push_back("Expected '(' after function name at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    std::vector<std::unique_ptr<Token>> parameterTokens;
    if (this->peek() != TokenName::ParenthesisClose) {
        while (true) {
            auto parameterToken = this->expectAndAdvance(TokenName::Identifier);
            if (!parameterToken) {
                this->errorMessages.push_back("Expected identifier in parameter list at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                return nullptr;
            }
            parameterTokens.push_back(std::make_unique<Token>(parameterToken.value()));
            if (this->peek() == TokenName::Comma) {
                this->expectAndAdvance(TokenName::Comma);
            } else {
                break;
            }
        }
    }
    if (!this->expectAndAdvance(TokenName::ParenthesisClose)) {
        this->errorMessages.push_back("Expected ')' after parameter list at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    this->enterFunction();
    auto bodyNode = this->parseBlockStatement();
    this->exitFunction();
    if (!bodyNode) {
        this->errorMessages.push_back("Failed to parse function body at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto functionNameIdentifier = std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value()));
    std::vector<std::unique_ptr<IdentifierNode>> parameters;
    for (const auto& paramToken : parameterTokens) {
        auto identifierNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(*paramToken));
        parameters.push_back(std::move(identifierNode));
    }
    auto functionDeclaration = std::make_unique<FunctionDeclarationNode>(std::move(functionNameIdentifier), std::move(parameters), std::move(bodyNode));
    return functionDeclaration;
}

std::unique_ptr<IfStatementNode> Parser::parseIfStatement() {
    auto ifToken = this->expectAndAdvance(TokenName::KeywordIf);
    if (!ifToken) {
        this->errorMessages.push_back("Expected 'if' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::ParenthesisOpen)) {
        this->errorMessages.push_back("Expected '(' after 'if' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto conditionNode = this->parseExpression();
    if (!conditionNode) {
        this->errorMessages.push_back("Failed to parse if statement condition at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::ParenthesisClose)) {
        this->errorMessages.push_back("Expected ')' after if statement condition at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto thenBranchNode = this->parseBlockStatement();
    if (!thenBranchNode) {
        this->errorMessages.push_back("Failed to parse if statement then branch at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    std::unique_ptr<BlockStatementNode> elseBranchNode = nullptr;
    if (this->peek() == TokenName::KeywordElse) {
        this->expectAndAdvance(TokenName::KeywordElse);
        elseBranchNode = this->parseBlockStatement();
        if (!elseBranchNode) {
            this->errorMessages.push_back("Failed to parse if statement else branch at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
            return nullptr;
        }
    }
    auto ifStatement = std::make_unique<IfStatementNode>(std::move(conditionNode), std::move(thenBranchNode), std::move(elseBranchNode));
    return ifStatement;
}

std::unique_ptr<WhileStatementNode> Parser::parseWhileStatement() {
    auto whileToken = this->expectAndAdvance(TokenName::KeywordWhile);
    if (!whileToken) {
        this->errorMessages.push_back("Expected 'while' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::ParenthesisOpen)) {
        this->errorMessages.push_back("Expected '(' after 'while' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto conditionNode = this->parseExpression();
    if (!conditionNode) {
        this->errorMessages.push_back("Failed to parse while statement condition at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::ParenthesisClose)) {
        this->errorMessages.push_back("Expected ')' after while statement condition at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    this->enterLoop();
    auto bodyNode = this->parseBlockStatement();
    this->exitLoop();
    if (!bodyNode) {
        this->errorMessages.push_back("Failed to parse while statement body at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto whileStatement = std::make_unique<WhileStatementNode>(std::move(conditionNode), std::move(bodyNode));
    return whileStatement;
}

std::unique_ptr<ReturnStatementNode> Parser::parseReturnStatement() {
    this->expectAndAdvance(TokenName::KeywordReturn);
    std::unique_ptr<ExpressionNode> expressionNode = nullptr;
    if (this->peek() != TokenName::Semicolon) {
        expressionNode = this->parseExpression();
        if (!expressionNode) {
            this->errorMessages.push_back("Failed to parse return statement expression at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
            return nullptr;
        }
    }
    this->expectAndAdvance(TokenName::Semicolon);
    auto returnStatement = std::make_unique<ReturnStatementNode>(std::move(expressionNode));
    return returnStatement;
}

std::unique_ptr<BlockStatementNode> Parser::parseBlockStatement() {
    this->expectAndAdvance(TokenName::BraceOpen);
    this->enterBlock();
    auto programNode = Parser::parseProgram();
    this->exitBlock();
    if (!programNode) {
        this->errorMessages.push_back("Failed to parse block statement program node at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto blockStatement = std::make_unique<BlockStatementNode>(std::move(programNode));
    return blockStatement;
}

std::unique_ptr<BreakStatementNode> Parser::parseBreakStatement() {
    if (!this->expectAndAdvance(TokenName::KeywordBreak)) {
        this->errorMessages.push_back("Expected 'break' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::Semicolon)) {
        this->errorMessages.push_back("Expected ';' after 'break' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto breakStatement = std::make_unique<BreakStatementNode>();
    return breakStatement;
}

std::unique_ptr<ContinueStatementNode> Parser::parseContinueStatement() {
    if (!this->expectAndAdvance(TokenName::KeywordContinue)) {
        this->errorMessages.push_back("Expected 'continue' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::Semicolon)) {
        this->errorMessages.push_back("Expected ';' after 'continue' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto continueStatement = std::make_unique<ContinueStatementNode>();
    return continueStatement;
}

std::unique_ptr<FunctionCallExpressionNode> Parser::parseFunctionCallExpression() {
    auto identifierToken = this->expectAndAdvance(TokenName::Identifier);
    if (!identifierToken) {
        this->errorMessages.push_back("Expected function name identifier at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::ParenthesisOpen)) {
        this->errorMessages.push_back("Expected '(' after function name at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
    if (this->peek() != TokenName::ParenthesisClose) {
        while (true) {
            auto argumentNode = this->parseExpression();
            if (!argumentNode) {
                this->errorMessages.push_back("Failed to parse function call argument at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                return nullptr;
            }
            arguments.push_back(std::move(argumentNode));
            if (this->peek() == TokenName::Comma) {
                this->expectAndAdvance(TokenName::Comma);
            } else {
                break;
            }
        }
    }
    if (!this->expectAndAdvance(TokenName::ParenthesisClose)) {
        this->errorMessages.push_back("Expected ')' after function call arguments at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto identifierNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value()));
    auto functionCallExpression = std::make_unique<FunctionCallExpressionNode>(std::move(identifierNode), std::move(arguments));
    return functionCallExpression;
}

std::unique_ptr<FunctionCallStatementNode> Parser::parseFunctionCallStatement() {
    auto functionCallExpressionNode = this->parseFunctionCallExpression();
    if (!functionCallExpressionNode) {
        this->errorMessages.push_back("Failed to parse function call expression at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::Semicolon)) {
        this->errorMessages.push_back("Expected ';' after function call expression at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto functionCallStatement = std::make_unique<FunctionCallStatementNode>(std::move(functionCallExpressionNode));
    return functionCallStatement;
}

std::unique_ptr<AssignmentStatementNode> Parser::parseAssignmentStatement() {
    auto identifierToken = this->expectAndAdvance(TokenName::Identifier);
    if (!identifierToken) {
        this->errorMessages.push_back("Expected identifier at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::Equal)) {
        this->errorMessages.push_back("Expected '=' after identifier at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto expressionNode = this->parseExpression();
    if (!expressionNode) {
        this->errorMessages.push_back("Failed to parse assignment expression at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::Semicolon)) {
        this->errorMessages.push_back("Expected ';' after assignment expression at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto identifierNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value()));
    auto assignmentExpression = std::make_unique<AssignmentExpressionNode>(std::move(identifierNode), std::move(expressionNode));
    auto assignmentStatement = std::make_unique<AssignmentStatementNode>(std::move(assignmentExpression));
    return assignmentStatement;
}

std::unique_ptr<AssignmentExpressionNode> Parser::parseAssignmentExpression() {
    auto identifierToken = this->expectAndAdvance(TokenName::Identifier);
    if (!identifierToken) {
        this->errorMessages.push_back("Expected identifier at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenName::Equal)) {
        this->errorMessages.push_back("Expected '=' after identifier at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto expressionNode = this->parseExpression();
    if (!expressionNode) {
        this->errorMessages.push_back("Failed to parse assignment expression at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    auto identifierNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value()));
    auto assignmentExpression = std::make_unique<AssignmentExpressionNode>(std::move(identifierNode), std::move(expressionNode));
    return assignmentExpression;
}

std::unique_ptr<PrimaryExpressionNode> Parser::parsePrimaryExpression() {
    Token token = this->peek();
    switch (token.getTokenName()) {
        case TokenName::Identifier: {
            if (this->peek(1) == TokenName::ParenthesisOpen) {
                auto functionCallExpression = this->parseFunctionCallExpression();
                if (!functionCallExpression) {
                    this->errorMessages.push_back("Failed to parse function call expression at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                    return nullptr;
                }
                return functionCallExpression;
            }
            this->expectAndAdvance(TokenName::Identifier);
            auto identifierNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(token));
            return identifierNode;
        }
        case TokenName::BooleanLiteral: {
            auto booleanLiteralTokenOpt = this->expectAndAdvance(TokenName::BooleanLiteral);
            if (!booleanLiteralTokenOpt) {
                // unreachable
                this->errorMessages.push_back("Expected boolean literal at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                return nullptr;
            }
            std::unique_ptr<BooleanLiteralNode> booleanLiteralNode = std::make_unique<BooleanLiteralNode>(std::make_unique<Token>(booleanLiteralTokenOpt.value()));
            return booleanLiteralNode;
        }
        case TokenName::IntegerLiteral: {
            auto integerLiteralTokenOpt = this->expectAndAdvance(TokenName::IntegerLiteral);
            if (!integerLiteralTokenOpt) {
                // unreachable
                this->errorMessages.push_back("Expected integer literal at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                return nullptr;
            }
            std::unique_ptr<NumberLiteralNode> integerLiteralNode = std::make_unique<NumberLiteralNode>(std::make_unique<Token>(integerLiteralTokenOpt.value()));
            return integerLiteralNode;
        }
        case TokenName::StringLiteral: {
            auto stringLiteralTokenOpt = this->expectAndAdvance(TokenName::StringLiteral);
            if (!stringLiteralTokenOpt) {
                // unreachable
                this->errorMessages.push_back("Expected string literal at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                return nullptr;
            }
            std::unique_ptr<StringLiteralNode> stringLiteralNode = std::make_unique<StringLiteralNode>(std::make_unique<Token>(stringLiteralTokenOpt.value()));
            return stringLiteralNode;
        }
        case TokenName::Not: {
            auto operatorTokenOpt = this->expectAndAdvance(TokenName::Not);
            if (!operatorTokenOpt) {
                // unreachable
                this->errorMessages.push_back("Expected '!' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                return nullptr;
            }
            auto operandNode = this->parsePrimaryExpression();
            if (!operandNode) {
                this->errorMessages.push_back("Failed to parse operand of '!' operator at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                return nullptr;
            }
            auto unaryOperatorNode = std::make_unique<UnaryOperatorExpressionNode>(std::move(operandNode), std::make_unique<Token>(operatorTokenOpt.value()));
            return unaryOperatorNode;
        }
        case TokenName::Minus: {
            auto operatorTokenOpt = this->expectAndAdvance(TokenName::Minus);
            if (!operatorTokenOpt) {
                // unreachable
                this->errorMessages.push_back("Expected '-' at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                return nullptr;
            }
            auto operandNode = this->parsePrimaryExpression();
            if (!operandNode) {
                this->errorMessages.push_back("Failed to parse operand of '-' operator at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                return nullptr;
            }
            auto unaryOperatorNode = std::make_unique<UnaryOperatorExpressionNode>(std::move(operandNode), std::make_unique<Token>(operatorTokenOpt.value()));
            return unaryOperatorNode;
        }
        default:
            this->errorMessages.push_back("Unexpected token '" + token.getSourceString() + "' at line " + std::to_string(token.getLine()) + ", column " + std::to_string(token.getColumn()));
            return nullptr;
    }
}

std::unique_ptr<ExpressionNode> Parser::parseExpressionClimbing (std::unique_ptr<ExpressionNode> lhs, int minPrecedence = 0) {
    auto lookahead = this->peek();
    while (IS_TOKENNAME_OPERATOR(lookahead.getTokenName()) && getPrecedence(lookahead.getTokenName()) >= minPrecedence) {
        auto operatorTokenOpt = this->expectAndAdvance(lookahead.getTokenName());
        if (!operatorTokenOpt) {
            // unreachable
            this->errorMessages.push_back("Expected operator at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
            return nullptr;
        }
        std::unique_ptr<ExpressionNode> rhs = this->parsePrimaryExpression();
        if (!rhs) {
            this->errorMessages.push_back("Failed to parse right-hand side expression at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
            return nullptr;
        }
        lookahead = this->peek();
        while (
            (IS_TOKENNAME_BINARY_OPERATOR(lookahead.getTokenName()) && getPrecedence(lookahead.getTokenName()) > getPrecedence(operatorTokenOpt.value().getTokenName()))
            ||
            (IS_TOKENNAME_BINARY_OPERATOR(lookahead.getTokenName()) && getPrecedence(lookahead.getTokenName()) == getPrecedence(operatorTokenOpt.value().getTokenName()) && getAssociativity(lookahead.getTokenName()) == RIGHT_ASSOCIATIVE)
        ) {
            auto nextPrecedenceAddition = getPrecedence(lookahead.getTokenName()) > getPrecedence(operatorTokenOpt.value().getTokenName()) ? 1 : 0;
            rhs = this->parseExpressionClimbing(std::move(rhs), getPrecedence(operatorTokenOpt.value().getTokenName()) + nextPrecedenceAddition);
            if (!rhs) {
                this->errorMessages.push_back("Failed to parse right-hand side expression at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
                return nullptr;
            }
            lookahead = this->peek(1);
        }
        if (!IS_TOKENNAME_BINARY_OPERATOR(operatorTokenOpt.value().getTokenName())) {
            this->errorMessages.push_back("Expected binary operator at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
            return nullptr;
        }
        lhs = std::make_unique<BinaryOperatorExpressionNode>(std::move(lhs), std::move(rhs), std::make_unique<Token>(operatorTokenOpt.value()));
    }
    return lhs;
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() {
    auto token = this->peek();
    if (this->peek(1).getTokenName() == TokenName::Equal) {
        auto assignmentExpressionNode = this->parseAssignmentExpression();
        if (!assignmentExpressionNode) {
            this->errorMessages.push_back("Failed to parse assignment expression at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
            return nullptr;
        }
        return assignmentExpressionNode;
    }
    auto primaryExpressionNode = this->parsePrimaryExpression();
    if (!primaryExpressionNode) {
        this->errorMessages.push_back("Failed to parse primary expression at line " + std::to_string(this->peek().getLine()) + ", column " + std::to_string(this->peek().getColumn()));
        return nullptr;
    }
    return this->parseExpressionClimbing(std::move(primaryExpressionNode), 0);
}