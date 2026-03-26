#include "parser/parser.hpp"
#include "parser/node.hpp"
#include "lexer/token.hpp"

#include <memory>
#include <optional>
#include <string>
#include <print>

Token Parser::peek(int offset) const {
    if (this->index + offset >= this->tokens.size()) {
        return Token("", 0, 0, 0, TokenKind::OutOfRange);
    }
    return (this->tokens)[this->index + offset];
}

bool Parser::isPastTokensEnd() const {
    return this->index >= this->tokens.size();
}

std::optional<Token> Parser::expectAndAdvance(const TokenKind& expectedTokenName) {
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

void Parser::addErrorMessageParseFailure(const std::string& failedToParse) {
    this->errorMessages.push_back("Failed to parse " + failedToParse + " at line " + std::to_string(this->peek().getFirstLine()) + ", column " + std::to_string(this->peek().getFirstColumn()));
}

void Parser::addErrorMessageExpected(const std::string& expected) {
    this->errorMessages.push_back("Expected '" + expected + "' but found '" + this->peek().getSourceString() + "' at line " + std::to_string(this->peek().getFirstLine()) + ", column " + std::to_string(this->peek().getFirstColumn()));
}

void Parser::addErrorMessageUnexpected(const std::string& unexpected) {
    this->errorMessages.push_back("Unexpected '" + unexpected + "' at line " + std::to_string(this->peek().getFirstLine()) + ", column " + std::to_string(this->peek().getFirstColumn()));
}

std::unique_ptr<Node> Parser::parse() {
    auto program = this->parseProgram();
    if (!program) {
        return std::make_unique<InvalidNode>();
    }
    return program;
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    std::unique_ptr<ProgramNode> program = std::make_unique<ProgramNode>();
    while (!this->isPastTokensEnd()) {
        if (insideBlock && this->peek() == TokenKind::BraceClose) {
            this->expectAndAdvance(TokenKind::BraceClose);
            return program;
        }
        switch (this->peek().getTokenName()) {
            case TokenKind::KeywordVar: {
                auto node = this->parseVariableDeclaration();
                if (!node) {
                    this->addErrorMessageParseFailure("variable declaration");
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenKind::KeywordFunction: {
                auto node = this->parseFunctionDeclaration();
                if (!node) {
                    this->addErrorMessageParseFailure("function declaration");
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenKind::KeywordIf: {
                auto node = this->parseIfStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("if statement");
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenKind::KeywordReturn: {
                if (!this->insideFunction) {
                    this->addErrorMessageUnexpected("'return' statement outside of function");
                    return program;
                }
                auto node = this->parseReturnStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("return statement");
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenKind::KeywordBreak: {
                if (!this->insideLoop) {
                    this->addErrorMessageUnexpected("'break' statement outside of while loop");
                    return program;
                }
                auto node = this->parseBreakStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("break statement");
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenKind::KeywordContinue: {
                if (!this->insideLoop) {
                    this->addErrorMessageUnexpected("'continue' statement outside of while loop");
                    return program;
                }
                auto node = this->parseContinueStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("continue statement");
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenKind::KeywordWhile: {
                auto node = this->parseWhileStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("while statement");
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenKind::KeywordLoop: {
                auto node = this->parseLoopStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("loop statement");
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenKind::BraceOpen: {
                auto node = this->parseBlockStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("block statement");
                    return program;
                }
                program->addNode(std::move(node));
                break;
            }
            case TokenKind::Identifier: {
                if (this->peek(1) == TokenKind::Equal) {
                    auto node = this->parseAssignmentStatement();
                    if (!node) {
                        this->addErrorMessageParseFailure("assignment statement");
                        return program;
                    }
                    program->addNode(std::move(node));
                    break;
                } else if (this->peek(1) == TokenKind::ParenthesisOpen) {
                    auto node = this->parseFunctionCallStatement();
                    if (!node) {
                        this->addErrorMessageParseFailure("function call statement");
                        return program;
                    }
                    program->addNode(std::move(node));
                    break;
                } else {
                    this->addErrorMessageUnexpected("token '" + this->peek().getSourceString() + "'");
                    return program;
                }
            }
            default:
                this->addErrorMessageUnexpected("token '" + this->peek().getSourceString() + "'");
                return program;
        }
    }
    if (this->insideBlock) {
        this->addErrorMessageExpected("'}'");
        return program;
    }
    return program;
};

std::unique_ptr<VariableDeclarationNode> Parser::parseVariableDeclaration() {
    auto varToken = this->expectAndAdvance(TokenKind::KeywordVar);
    if (!varToken) {
        this->addErrorMessageExpected("'var'");
        return nullptr;
    }
    auto identifierToken = this->expectAndAdvance(TokenKind::Identifier);
    if (!identifierToken) {
        this->addErrorMessageExpected("identifier after 'var'");
        return nullptr;
    }
    if (this->peek() == TokenKind::Semicolon) {
        auto semicolonToken = this->expectAndAdvance(TokenKind::Semicolon);
        auto identiferNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value()));
        auto assignmentExpressionNode = std::make_unique<AssignmentExpressionNode>(std::move(identiferNode), nullptr);
        auto variableDeclaration = std::make_unique<VariableDeclarationNode>(std::move(assignmentExpressionNode));
        variableDeclaration->addToken(std::make_unique<Token>(varToken.value()));
        variableDeclaration->addToken(std::make_unique<Token>(identifierToken.value()));
        variableDeclaration->addToken(std::make_unique<Token>(semicolonToken.value()));
        return variableDeclaration;
    }
    std::unique_ptr<ExpressionNode> node;
    if (this->expectAndAdvance(TokenKind::Equal)) {
        node = this->parseExpression();
    }
    if (!node) {
        this->addErrorMessageExpected("expression after variable declaration");
        return nullptr;
    }
    this->expectAndAdvance(TokenKind::Semicolon);
    auto identifierNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value()));
    auto assignmentExpression = std::make_unique<AssignmentExpressionNode>(std::move(identifierNode), std::move(node));
    auto variableDeclaration = std::make_unique<VariableDeclarationNode>(std::move(assignmentExpression));
    return variableDeclaration;
}

std::unique_ptr<FunctionDeclarationNode> Parser::parseFunctionDeclaration() {
    auto functionToken = this->expectAndAdvance(TokenKind::KeywordFunction);
    if (!functionToken) {
        this->addErrorMessageExpected("'function'");
        return nullptr;
    }
    auto identifierToken = this->expectAndAdvance(TokenKind::Identifier);
    if (!identifierToken) {
        this->addErrorMessageExpected("identifier after 'function'");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::ParenthesisOpen)) {
        this->addErrorMessageExpected(" '(' after function name");
        return nullptr;
    }
    std::vector<std::unique_ptr<Token>> parameterTokens;
    if (this->peek() != TokenKind::ParenthesisClose) {
        while (true) {
            auto parameterToken = this->expectAndAdvance(TokenKind::Identifier);
            if (!parameterToken) {
                this->addErrorMessageExpected("identifier in parameter list");
                return nullptr;
            }
            parameterTokens.push_back(std::make_unique<Token>(parameterToken.value()));
            if (this->peek() == TokenKind::Comma) {
                this->expectAndAdvance(TokenKind::Comma);
            } else {
                break;
            }
        }
    }
    if (!this->expectAndAdvance(TokenKind::ParenthesisClose)) {
        this->addErrorMessageExpected("')' after parameter list");
        return nullptr;
    }
    this->enterFunction();
    auto bodyNode = this->parseBlockStatement();
    this->exitFunction();
    if (!bodyNode) {
        this->addErrorMessageParseFailure("function body");
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
    auto ifToken = this->expectAndAdvance(TokenKind::KeywordIf);
    if (!ifToken) {
        this->addErrorMessageExpected("'if'");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::ParenthesisOpen)) {
        this->addErrorMessageExpected(" '(' after 'if'");
        return nullptr;
    }
    auto conditionNode = this->parseExpression();
    if (!conditionNode) {
        this->addErrorMessageParseFailure("if statement condition");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::ParenthesisClose)) {
        this->addErrorMessageExpected("')' after if statement condition");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::KeywordThen)) {
        this->addErrorMessageExpected("'then' after if statement condition");
        return nullptr;
    }
    auto thenBranchNode = this->parseBlockStatement();
    if (!thenBranchNode) {
        this->addErrorMessageParseFailure("if statement then branch");
        return nullptr;
    }
    std::unique_ptr<BlockStatementNode> elseBranchNode = nullptr;
    if (this->peek() == TokenKind::KeywordElse) {
        this->expectAndAdvance(TokenKind::KeywordElse);
        elseBranchNode = this->parseBlockStatement();
        if (!elseBranchNode) {
            this->addErrorMessageParseFailure("if statement else branch");
            return nullptr;
        }
    }
    auto ifStatement = std::make_unique<IfStatementNode>(std::move(conditionNode), std::move(thenBranchNode), std::move(elseBranchNode));
    return ifStatement;
}

std::unique_ptr<WhileStatementNode> Parser::parseWhileStatement() {
    auto whileToken = this->expectAndAdvance(TokenKind::KeywordWhile);
    if (!whileToken) {
        this->addErrorMessageExpected("'while'");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::ParenthesisOpen)) {
        this->addErrorMessageExpected(" '(' after 'while'");
        return nullptr;
    }
    auto conditionNode = this->parseExpression();
    if (!conditionNode) {
        this->addErrorMessageParseFailure("while statement condition");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::ParenthesisClose)) {
        this->addErrorMessageExpected("')' after while statement condition");
        return nullptr;
    }
    this->enterLoop();
    auto bodyNode = this->parseBlockStatement();
    this->exitLoop();
    if (!bodyNode) {
        this->addErrorMessageParseFailure("while statement body");
        return nullptr;
    }
    auto whileStatement = std::make_unique<WhileStatementNode>(std::move(conditionNode), std::move(bodyNode));
    return whileStatement;
}

std::unique_ptr<LoopStatementNode> Parser::parseLoopStatement() {
    auto loopToken = this->expectAndAdvance(TokenKind::KeywordLoop);
    if (!loopToken) {
        this->addErrorMessageExpected("'loop'");
        return nullptr;
    }
    this->enterLoop();
    auto bodyNode = this->parseBlockStatement();
    this->exitLoop();
    if (!bodyNode) {
        this->addErrorMessageParseFailure("loop statement body");
        return nullptr;
    }
    auto loopStatement = std::make_unique<LoopStatementNode>(std::move(bodyNode));
    return loopStatement;
}

std::unique_ptr<ReturnStatementNode> Parser::parseReturnStatement() {
    this->expectAndAdvance(TokenKind::KeywordReturn);
    std::unique_ptr<ExpressionNode> expressionNode = nullptr;
    if (this->peek() != TokenKind::Semicolon) {
        expressionNode = this->parseExpression();
        if (!expressionNode) {
            this->addErrorMessageParseFailure("return statement expression");
            return nullptr;
        }
    }
    this->expectAndAdvance(TokenKind::Semicolon);
    auto returnStatement = std::make_unique<ReturnStatementNode>(std::move(expressionNode));
    return returnStatement;
}

std::unique_ptr<BlockStatementNode> Parser::parseBlockStatement() {
    this->expectAndAdvance(TokenKind::BraceOpen);
    this->enterBlock();
    auto programNode = Parser::parseProgram();
    this->exitBlock();
    if (!programNode) {
        this->addErrorMessageParseFailure("block statement program node");
        return nullptr;
    }
    auto blockStatement = std::make_unique<BlockStatementNode>(std::move(programNode));
    return blockStatement;
}

std::unique_ptr<BreakStatementNode> Parser::parseBreakStatement() {
    if (!this->expectAndAdvance(TokenKind::KeywordBreak)) {
        this->addErrorMessageExpected("'break'");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::Semicolon)) {
        this->addErrorMessageExpected("';' after 'break'");
        return nullptr;
    }
    auto breakStatement = std::make_unique<BreakStatementNode>();
    return breakStatement;
}

std::unique_ptr<ContinueStatementNode> Parser::parseContinueStatement() {
    if (!this->expectAndAdvance(TokenKind::KeywordContinue)) {
        this->addErrorMessageExpected("'continue'");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::Semicolon)) {
        this->addErrorMessageExpected("';' after 'continue'");
        return nullptr;
    }
    auto continueStatement = std::make_unique<ContinueStatementNode>();
    return continueStatement;
}

std::unique_ptr<FunctionCallExpressionNode> Parser::parseFunctionCallExpression() {
    auto identifierToken = this->expectAndAdvance(TokenKind::Identifier);
    if (!identifierToken) {
        this->addErrorMessageExpected("function name identifier");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::ParenthesisOpen)) {
        this->addErrorMessageExpected("Expected '(' after function name");
        return nullptr;
    }
    std::vector<std::unique_ptr<ExpressionNode>> arguments;
    if (this->peek() != TokenKind::ParenthesisClose) {
        while (true) {
            auto argumentNode = this->parseExpression();
            if (!argumentNode) {
                this->addErrorMessageParseFailure("function call argument");
                return nullptr;
            }
            arguments.push_back(std::move(argumentNode));
            if (this->peek() == TokenKind::Comma) {
                this->expectAndAdvance(TokenKind::Comma);
            } else {
                break;
            }
        }
    }
    if (!this->expectAndAdvance(TokenKind::ParenthesisClose)) {
        this->addErrorMessageExpected("')' after function call arguments");
        return nullptr;
    }
    auto identifierNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value()));
    auto functionCallExpression = std::make_unique<FunctionCallExpressionNode>(std::move(identifierNode), std::move(arguments));
    return functionCallExpression;
}

std::unique_ptr<FunctionCallStatementNode> Parser::parseFunctionCallStatement() {
    auto functionCallExpressionNode = this->parseFunctionCallExpression();
    if (!functionCallExpressionNode) {
        this->addErrorMessageParseFailure("function call expression");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::Semicolon)) {
        this->addErrorMessageExpected("';' after function call expression");
        return nullptr;
    }
    auto functionCallStatement = std::make_unique<FunctionCallStatementNode>(std::move(functionCallExpressionNode));
    return functionCallStatement;
}

std::unique_ptr<AssignmentStatementNode> Parser::parseAssignmentStatement() {
    auto identifierToken = this->expectAndAdvance(TokenKind::Identifier);
    if (!identifierToken) {
        this->addErrorMessageExpected("identifier");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::Equal)) {
        this->addErrorMessageExpected("Expected '=' after identifier");
        return nullptr;
    }
    auto expressionNode = this->parseExpression();
    if (!expressionNode) {
        this->addErrorMessageParseFailure("assignment expression");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::Semicolon)) {
        this->addErrorMessageExpected("';' after assignment expression");
        return nullptr;
    }
    auto identifierNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value()));
    auto assignmentExpression = std::make_unique<AssignmentExpressionNode>(std::move(identifierNode), std::move(expressionNode));
    auto assignmentStatement = std::make_unique<AssignmentStatementNode>(std::move(assignmentExpression));
    return assignmentStatement;
}

std::unique_ptr<AssignmentExpressionNode> Parser::parseAssignmentExpression() {
    auto identifierToken = this->expectAndAdvance(TokenKind::Identifier);
    if (!identifierToken) {
        this->addErrorMessageExpected("identifier");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::Equal)) {
        this->addErrorMessageExpected("'=' after identifier");
        return nullptr;
    }
    auto expressionNode = this->parseExpression();
    if (!expressionNode) {
        this->addErrorMessageParseFailure("assignment expression");
        return nullptr;
    }
    auto identifierNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(identifierToken.value()));
    auto assignmentExpression = std::make_unique<AssignmentExpressionNode>(std::move(identifierNode), std::move(expressionNode));
    return assignmentExpression;
}

std::unique_ptr<PrimaryExpressionNode> Parser::parsePrimaryExpression() {
    Token token = this->peek();
    switch (token.getTokenName()) {
        case TokenKind::Identifier: {
            if (this->peek(1) == TokenKind::ParenthesisOpen) {
                auto functionCallExpression = this->parseFunctionCallExpression();
                if (!functionCallExpression) {
                    this->addErrorMessageParseFailure("function call expression");
                    return nullptr;
                }
                return functionCallExpression;
            }
            this->expectAndAdvance(TokenKind::Identifier);
            auto identifierNode = std::make_unique<IdentifierNode>(std::make_unique<Token>(token));
            return identifierNode;
        }
        case TokenKind::LiteralBoolean: {
            auto booleanLiteralTokenOpt = this->expectAndAdvance(TokenKind::LiteralBoolean);
            if (!booleanLiteralTokenOpt) {
                // unreachable
                this->addErrorMessageExpected("boolean literal");
                return nullptr;
            }
            std::unique_ptr<BooleanLiteralNode> booleanLiteralNode = std::make_unique<BooleanLiteralNode>(std::make_unique<Token>(booleanLiteralTokenOpt.value()));
            return booleanLiteralNode;
        }
        case TokenKind::LiteralInteger: {
            auto integerLiteralTokenOpt = this->expectAndAdvance(TokenKind::LiteralInteger);
            if (!integerLiteralTokenOpt) {
                // unreachable
                this->addErrorMessageExpected("integer literal");
                return nullptr;
            }
            std::unique_ptr<NumberLiteralNode> integerLiteralNode = std::make_unique<NumberLiteralNode>(std::make_unique<Token>(integerLiteralTokenOpt.value()));
            return integerLiteralNode;
        }
        case TokenKind::LiteralString: {
            auto stringLiteralTokenOpt = this->expectAndAdvance(TokenKind::LiteralString);
            if (!stringLiteralTokenOpt) {
                // unreachable
                this->addErrorMessageExpected("string literal");
                return nullptr;
            }
            std::unique_ptr<StringLiteralNode> stringLiteralNode = std::make_unique<StringLiteralNode>(std::make_unique<Token>(stringLiteralTokenOpt.value()));
            return stringLiteralNode;
        }
        case TokenKind::LiteralEmpty: {
            auto emptyLiteralTokenOpt = this->expectAndAdvance(TokenKind::LiteralEmpty);
            if (!emptyLiteralTokenOpt) {
                // unreachable
                this->addErrorMessageExpected("empty literal");
                return nullptr;
            }
            std::unique_ptr<EmptyLiteralNode> emptyLiteralNode = std::make_unique<EmptyLiteralNode>(std::make_unique<Token>(emptyLiteralTokenOpt.value()));
            return emptyLiteralNode;
        }
        case TokenKind::Not: {
            auto operatorTokenOpt = this->expectAndAdvance(TokenKind::Not);
            if (!operatorTokenOpt) {
                // unreachable
                this->addErrorMessageExpected("'!' operator");
                return nullptr;
            }
            auto operandNode = this->parsePrimaryExpression();
            if (!operandNode) {
                this->addErrorMessageParseFailure("operand of '!' operator");
                return nullptr;
            }
            auto unaryOperatorNode = std::make_unique<UnaryOperatorExpressionNode>(std::move(operandNode), std::make_unique<Token>(operatorTokenOpt.value()));
            return unaryOperatorNode;
        }
        case TokenKind::Dash: {
            auto operatorTokenOpt = this->expectAndAdvance(TokenKind::Dash);
            if (!operatorTokenOpt) {
                // unreachable
                this->addErrorMessageExpected("'-' operator");
                return nullptr;
            }
            auto operandNode = this->parsePrimaryExpression();
            if (!operandNode) {
                this->addErrorMessageParseFailure("operand of '-' operator");
                return nullptr;
            }
            auto unaryOperatorNode = std::make_unique<UnaryOperatorExpressionNode>(std::move(operandNode), std::make_unique<Token>(operatorTokenOpt.value()));
            return unaryOperatorNode;
        }
        default:
            this->addErrorMessageUnexpected("token '" + token.getSourceString() + "'");
            return nullptr;
    }
}

std::unique_ptr<ExpressionNode> Parser::parseExpressionClimbing (std::unique_ptr<ExpressionNode> lhs, int minPrecedence = 0) {
    auto lookahead = this->peek();
    while (IS_TOKENNAME_OPERATOR(lookahead.getTokenName()) && getPrecedence(lookahead.getTokenName()) >= minPrecedence) {
        auto operatorTokenOpt = this->expectAndAdvance(lookahead.getTokenName());
        if (!operatorTokenOpt) {
            // unreachable
            this->addErrorMessageExpected("operator");
            return nullptr;
        }
        std::unique_ptr<ExpressionNode> rhs = this->parsePrimaryExpression();
        if (!rhs) {
            this->addErrorMessageParseFailure("right-hand side expression");
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
                this->addErrorMessageParseFailure("right-hand side expression");
                return nullptr;
            }
            lookahead = this->peek(1);
        }
        if (!IS_TOKENNAME_BINARY_OPERATOR(operatorTokenOpt.value().getTokenName())) {
            this->addErrorMessageExpected("binary operator");
            return nullptr;
        }
        lhs = std::make_unique<BinaryOperatorExpressionNode>(std::move(lhs), std::move(rhs), std::make_unique<Token>(operatorTokenOpt.value()));
    }
    return lhs;
}

std::unique_ptr<IfExpressionNode> Parser::parseIfExpression() {
    auto ifToken = this->expectAndAdvance(TokenKind::KeywordIf);
    if (!ifToken) {
        this->addErrorMessageExpected("'if' keyword");
        return nullptr;
    }
    auto conditionNode = this->parseExpression();
    if (!conditionNode) {
        this->addErrorMessageParseFailure("if expression condition");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::KeywordThen)) {
        this->addErrorMessageExpected("'then' keyword");
        return nullptr;
    }
    auto thenBranchNode = this->parseExpression();
    if (!thenBranchNode) {
        this->addErrorMessageParseFailure("if expression then branch");
        return nullptr;
    }
    if (!this->expectAndAdvance(TokenKind::KeywordElse)) {
        this->addErrorMessageExpected("'else' branch, as required in if expressions");
        return nullptr;
    }
    auto elseBranchNode = this->parseExpression();
    if (!elseBranchNode) {
        this->addErrorMessageParseFailure("if expression else branch");
        return nullptr;
    }
    auto ifExpression = std::make_unique<IfExpressionNode>(std::move(conditionNode), std::move(thenBranchNode), std::move(elseBranchNode));
    return ifExpression;
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() {
    auto token = this->peek();
    if (this->peek(1).getTokenName() == TokenKind::Equal) {
        auto assignmentExpressionNode = this->parseAssignmentExpression();
        if (!assignmentExpressionNode) {
            this->addErrorMessageParseFailure("assignment expression");
            return nullptr;
        }
        return assignmentExpressionNode;
    }
    if (token == TokenKind::KeywordIf) {
        auto ifExpressionNode = this->parseIfExpression();
        if (!ifExpressionNode) {
            this->addErrorMessageParseFailure("if expression");
            return nullptr;
        }
        return ifExpressionNode;
    }
    auto primaryExpressionNode = this->parsePrimaryExpression();
    if (!primaryExpressionNode) {
        this->addErrorMessageParseFailure("primary expression");
        return nullptr;
    }
    return this->parseExpressionClimbing(std::move(primaryExpressionNode), 0);
}