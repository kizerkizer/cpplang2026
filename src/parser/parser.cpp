#include "parser/parser.hpp"
#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnosticmessage.hpp"
#include "parser/node.hpp"
#include "lexer/token.hpp"

#include <memory>
#include <string>
#include <print>

Token Parser::peek(size_t offset) {
    /*if (this->isPastTokensEnd()) {
        return Token(this->source, "", SourceCodeLocationSpan(SourceCodeLocation(-1, -1, -1), SourceCodeLocation(-1, -1, -1)), TokenKind::OutOfRange);
    }*/
    int needed = offset - this->tokenBuffer.size();
    while (needed >= 0) {
        this->tokenBuffer.push_back(this->lexer->getNextNonTrivialToken());
        needed--;
    }
    return *this->tokenBuffer[offset]; // offset guaranteed to be < tokenBuffer.size()
}

bool Parser::isPastTokensEnd() const {
    return this->lexer->isDone();
}

std::unique_ptr<Token> Parser::consumeCurrentToken() {
    if (!this->tokenBuffer.empty()) {
        std::unique_ptr<Token> token = std::move(this->tokenBuffer.front());
        this->tokenBuffer.pop_front();
        this->index++;
        return token;
    }
    std::unique_ptr<Token> token = this->lexer->getNextNonTrivialToken();
    this->index++;
    return token;
}

std::unique_ptr<Token> Parser::expectAndAdvance(const TokenKind& expectedTokenName) {
    /*if (this->isPastTokensEnd()) {
        return nullptr;
    }*/
    auto token = this->peek();
    if (token != expectedTokenName) {
        return nullptr;
    }
    auto returnToken = this->consumeCurrentToken();
    return returnToken;
}

SourceCodeLocationSpan Parser::getCurrentSourceCodeLocationSpan() {
    auto token = this->peek();
    return token.getSourceCodeLocationSpan();
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
    this->diagnostics.addDiagnosticMessage(DiagnosticMessage(5, DiagnosticMessageKind::Error, DiagnosticMessageStage::Parser, this->peek().getSourceCodeLocationSpan(), this->source, "Failed to parse " + failedToParse));
}

void Parser::addErrorMessageExpected(const std::string& expected) {
    this->diagnostics.addDiagnosticMessage(DiagnosticMessage(6, DiagnosticMessageKind::Error, DiagnosticMessageStage::Parser, this->peek().getSourceCodeLocationSpan(), this->source, "Expected '" + expected + "' but found '" + std::string(this->peek().getSourceString()) + "'"));
}

void Parser::addErrorMessageUnexpected(const std::string& unexpected) {
    this->diagnostics.addDiagnosticMessage(DiagnosticMessage(6, DiagnosticMessageKind::Error, DiagnosticMessageStage::Parser, this->peek().getSourceCodeLocationSpan(), this->source, "Unexpected '" + unexpected + "'"));
}

std::unique_ptr<Node> Parser::parse() {
    auto program = this->parseProgram();
    if (!program) {
        return std::make_unique<InvalidNode>();
    }
    return program;
}

std::unique_ptr<ProgramNode> Parser::parseProgram() {
    auto executionListNode = this->parseExecutionList();
    auto programNode = std::make_unique<ProgramNode>(std::move(executionListNode), executionListNode->getSourceCodeLocationSpan());
    return programNode;
};

std::unique_ptr<ExecutionListNode> Parser::parseExecutionList() {
    std::vector<std::unique_ptr<Node>> children;
    while (!this->isPastTokensEnd()) {
        if (insideBlock && this->peek() == TokenKind::BraceClose) {
            goto makeExecutionList;
        }
        switch (this->peek().getTokenKind()) {
            case TokenKind::KeywordVar: {
                auto node = this->parseVariableDeclaration();
                if (!node) {
                    this->addErrorMessageParseFailure("variable declaration");
                    goto makeExecutionList;
                }
                children.push_back(std::move(node));
                break;
            }
            case TokenKind::KeywordFunction: {
                auto node = this->parseFunctionDeclaration();
                if (!node) {
                    this->addErrorMessageParseFailure("function declaration");
                    goto makeExecutionList;
                }
                children.push_back(std::move(node));
                break;
            }
            case TokenKind::KeywordIf: {
                auto node = this->parseIfStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("if statement");
                    goto makeExecutionList;
                }
                children.push_back(std::move(node));
                break;
            }
            case TokenKind::KeywordReturn: {
                if (!this->insideFunction) {
                    this->addErrorMessageUnexpected("'return' statement outside of function");
                    goto makeExecutionList;
                }
                auto node = this->parseReturnStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("return statement");
                    goto makeExecutionList;
                }
                children.push_back(std::move(node));
                break;
            }
            case TokenKind::KeywordBreak: {
                if (!this->insideLoop) {
                    this->addErrorMessageUnexpected("'break' statement outside of while loop");
                    goto makeExecutionList;
                }
                auto node = this->parseBreakStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("break statement");
                    goto makeExecutionList;
                }
                children.push_back(std::move(node));
                break;
            }
            case TokenKind::KeywordContinue: {
                if (!this->insideLoop) {
                    this->addErrorMessageUnexpected("'continue' statement outside of while loop");
                    goto makeExecutionList;
                }
                auto node = this->parseContinueStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("continue statement");
                    goto makeExecutionList;
                }
                children.push_back(std::move(node));
                break;
            }
            case TokenKind::KeywordWhile: {
                auto node = this->parseWhileStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("while statement");
                    goto makeExecutionList;
                }
                children.push_back(std::move(node));
                break;
            }
            case TokenKind::KeywordLoop: {
                auto node = this->parseLoopStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("loop statement");
                    goto makeExecutionList;
                }
                children.push_back(std::move(node));
                break;
            }
            case TokenKind::BraceOpen: { // TODO Allow stand-alone block statements?
                auto node = this->parseBlockStatement();
                if (!node) {
                    this->addErrorMessageParseFailure("block statement");
                    goto makeExecutionList;
                }
                children.push_back(std::move(node));
                break;
            }
            case TokenKind::Identifier: {
                if (this->peek(1) == TokenKind::Equal) {
                    auto node = this->parseAssignmentStatement();
                    if (!node) {
                        this->addErrorMessageParseFailure("assignment statement");
                        goto makeExecutionList;
                    }
                    children.push_back(std::move(node));
                    break;
                } else if (this->peek(1) == TokenKind::ParenthesisOpen) {
                    auto node = this->parseFunctionCallStatement();
                    if (!node) {
                        this->addErrorMessageParseFailure("function call statement");
                        goto makeExecutionList;
                    }
                    children.push_back(std::move(node));
                    break;
                } else {
                    this->addErrorMessageUnexpected("token '" + std::string(this->peek().getSourceString()) + "'");
                    goto makeExecutionList;
                }
            }
            case TokenKind::OutOfRange: {
                goto makeExecutionList;
            }
            default:
                this->addErrorMessageUnexpected("token '" + std::string(this->peek().getSourceString()) + "'");
                goto makeExecutionList;
        }
    }
    makeExecutionList:
        SourceCodeLocation start = children.size() > 0 ? children.front()->getSourceCodeLocationSpan().start : this->getCurrentSourceCodeLocationSpan().start;
        SourceCodeLocation end = children.size() > 0 ? children.back()->getSourceCodeLocationSpan().end : start;
        SourceCodeLocationSpan sourceCodeLocationSpan(start, end);
        auto executionListNode = std::make_unique<ExecutionListNode>(sourceCodeLocationSpan);
        for (auto& child : children) {
            executionListNode->addNode(std::move(child));
        }
        return executionListNode;
}

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
    std::unique_ptr<TypeExpressionNode> typeExpression = nullptr;
    if (this->peek() == TokenKind::Colon) {
        // parse type annotation
        auto colonToken = this->expectAndAdvance(TokenKind::Colon);
        typeExpression = this->parseTypeExpression();
        if (!typeExpression) {
            this->addErrorMessageParseFailure("type annotation");
            return nullptr;
        }
    }
    if (this->peek() == TokenKind::Semicolon) {
        auto semicolonToken = this->expectAndAdvance(TokenKind::Semicolon);
        auto identiferNode = std::make_unique<IdentifierNode>(std::move(identifierToken), identifierToken->getSourceCodeLocationSpan());
        auto sourceCodeLocationSpan = SourceCodeLocationSpan(varToken->getSourceCodeLocationSpan().start, semicolonToken->getSourceCodeLocationSpan().end);
        auto variableDeclaration = std::make_unique<VariableDeclarationNode>(std::move(identiferNode), std::move(typeExpression), nullptr, sourceCodeLocationSpan);
        return variableDeclaration;
    }
    std::unique_ptr<ExpressionNode> expressionNode;
    if (this->expectAndAdvance(TokenKind::Equal)) {
        expressionNode = this->parseExpression();
    }
    if (!expressionNode) {
        this->addErrorMessageExpected("expression after variable declaration");
        return nullptr;
    }
    // TODO eventually support comma-delimited declarations eg var foo = 5, bar = 6, ...;
    auto semicolonToken = this->expectAndAdvance(TokenKind::Semicolon);
    auto identifierNode = std::make_unique<IdentifierNode>(std::move(identifierToken), identifierToken->getSourceCodeLocationSpan());
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(varToken->getSourceCodeLocationSpan().start, semicolonToken->getSourceCodeLocationSpan().end);
    auto variableDeclaration = std::make_unique<VariableDeclarationNode>(std::move(identifierNode), std::move(typeExpression), std::move(expressionNode), sourceCodeLocationSpan);
    return variableDeclaration;
}

std::unique_ptr<IdentifierWithPossibleAnnotationNode> Parser::parseIdentifierWithPossibleAnnotation() {
    auto identifierToken = this->expectAndAdvance(TokenKind::Identifier);
    if (!identifierToken) {
        this->addErrorMessageExpected("identifier");
        return nullptr;
    }
    std::unique_ptr<TypeExpressionNode> annotation = nullptr;
    if (this->peek() == TokenKind::Colon) {
        this->expectAndAdvance(TokenKind::Colon);
        annotation = this->parseTypeExpression();
    }
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(identifierToken->getSourceCodeLocationSpan().start, annotation ? annotation->getSourceCodeLocationSpan().end : identifierToken->getSourceCodeLocationSpan().end);
    auto identifier = std::make_unique<IdentifierWithPossibleAnnotationNode>(std::move(identifierToken), std::move(annotation), sourceCodeLocationSpan);
    return identifier;
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
    std::vector<std::unique_ptr<IdentifierWithPossibleAnnotationNode>> parameters;
    while (this->peek() != TokenKind::ParenthesisClose) {
        std::unique_ptr<IdentifierWithPossibleAnnotationNode> identifier = this->parseIdentifierWithPossibleAnnotation();
        if (!identifier) {
            this->addErrorMessageParseFailure("function parameter");
            return nullptr;
        }
        parameters.push_back(std::move(identifier));
        if (this->peek() == TokenKind::Comma) {
            this->expectAndAdvance(TokenKind::Comma);
        } else {
            break;
        }
    }
    if (!this->expectAndAdvance(TokenKind::ParenthesisClose)) {
        this->addErrorMessageExpected("')' after parameter list");
        return nullptr;
    }
    std::unique_ptr<TypeExpressionNode> returnTypeExpression = nullptr;
    if (this->peek() == TokenKind::Colon) {
        this->expectAndAdvance(TokenKind::Colon);
        returnTypeExpression = this->parseTypeExpression();
        if (!returnTypeExpression) {
            this->addErrorMessageParseFailure("function return type annotation");
        }
    }
    this->enterFunction();
    auto bodyNode = this->parseBlockStatement();
    this->exitFunction();
    if (!bodyNode) {
        this->addErrorMessageParseFailure("function body");
        return nullptr;
    }
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(functionToken->getSourceCodeLocationSpan().start, bodyNode->getSourceCodeLocationSpan().end);  
    auto functionNameIdentifier = std::make_unique<IdentifierNode>(std::move(identifierToken), identifierToken->getSourceCodeLocationSpan());
    auto functionDeclaration = std::make_unique<FunctionDeclarationNode>(std::move(functionNameIdentifier), std::move(parameters), std::move(bodyNode), std::move(returnTypeExpression), sourceCodeLocationSpan);
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
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(ifToken->getSourceCodeLocationSpan().start, (elseBranchNode ? elseBranchNode->getSourceCodeLocationSpan().end : thenBranchNode->getSourceCodeLocationSpan().end));
    auto ifStatement = std::make_unique<IfStatementNode>(std::move(conditionNode), std::move(thenBranchNode), std::move(elseBranchNode), sourceCodeLocationSpan);
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
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(whileToken->getSourceCodeLocationSpan().start, bodyNode->getSourceCodeLocationSpan().end);
    auto whileStatement = std::make_unique<WhileStatementNode>(std::move(conditionNode), std::move(bodyNode), sourceCodeLocationSpan);
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
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(loopToken->getSourceCodeLocationSpan().start, bodyNode->getSourceCodeLocationSpan().end);
    auto loopStatement = std::make_unique<LoopStatementNode>(std::move(bodyNode), sourceCodeLocationSpan);
    return loopStatement;
}

std::unique_ptr<ReturnStatementNode> Parser::parseReturnStatement() {
    auto returnToken = this->expectAndAdvance(TokenKind::KeywordReturn);
    if (!returnToken) {
        this->addErrorMessageExpected("'return'");
        return nullptr;
    }
    std::unique_ptr<ExpressionNode> expressionNode = nullptr;
    if (this->peek() != TokenKind::Semicolon) {
        expressionNode = this->parseExpression();
        if (!expressionNode) {
            this->addErrorMessageParseFailure("return statement expression");
            return nullptr;
        }
    }
    this->expectAndAdvance(TokenKind::Semicolon);
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(returnToken->getSourceCodeLocationSpan().start, (expressionNode ? expressionNode->getSourceCodeLocationSpan().end : returnToken->getSourceCodeLocationSpan().end));
    auto returnStatement = std::make_unique<ReturnStatementNode>(std::move(expressionNode), sourceCodeLocationSpan);
    return returnStatement;
}

std::unique_ptr<BlockStatementNode> Parser::parseBlockStatement() {
    auto braceOpenToken = this->expectAndAdvance(TokenKind::BraceOpen);
    if (!braceOpenToken) {
        this->addErrorMessageExpected("'{'");
        return nullptr;
    }
    this->enterBlock();
    //auto programNode = Parser::parseProgram(false);
    auto executionListNode = this->parseExecutionList();
    this->exitBlock();
    if (!executionListNode) {
        this->addErrorMessageParseFailure("block statement execution list node");
        return nullptr;
    }
    auto braceCloseToken = this->expectAndAdvance(TokenKind::BraceClose);
    if (!braceCloseToken) {
        this->addErrorMessageExpected("'}' after block statement body");
        return nullptr;
    }
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(braceOpenToken->getSourceCodeLocationSpan().start, braceCloseToken->getSourceCodeLocationSpan().end);
    auto blockStatement = std::make_unique<BlockStatementNode>(std::move(executionListNode), sourceCodeLocationSpan);
    return blockStatement;
}

std::unique_ptr<BreakStatementNode> Parser::parseBreakStatement() {
    auto breakToken = this->expectAndAdvance(TokenKind::KeywordBreak);
    if (!breakToken) {
        this->addErrorMessageExpected("'break'");
        return nullptr;
    }
    auto semicolonToken = this->expectAndAdvance(TokenKind::Semicolon);
    if (!semicolonToken) {
        this->addErrorMessageExpected("';' after 'break'");
        return nullptr;
    }
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(breakToken->getSourceCodeLocationSpan().start, semicolonToken->getSourceCodeLocationSpan().end);
    auto breakStatement = std::make_unique<BreakStatementNode>(sourceCodeLocationSpan);
    return breakStatement;
}

std::unique_ptr<ContinueStatementNode> Parser::parseContinueStatement() {
    auto continueToken = this->expectAndAdvance(TokenKind::KeywordContinue);
    if (!continueToken) {
        this->addErrorMessageExpected("'continue'");
        return nullptr;
    }
    auto semicolonToken = this->expectAndAdvance(TokenKind::Semicolon);
    if (!semicolonToken) {
        this->addErrorMessageExpected("';' after 'continue'");
        return nullptr;
    }
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(continueToken->getSourceCodeLocationSpan().start, semicolonToken->getSourceCodeLocationSpan().end);
    auto continueStatement = std::make_unique<ContinueStatementNode>(sourceCodeLocationSpan);
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
    auto parenthesisCloseToken = this->expectAndAdvance(TokenKind::ParenthesisClose);
    if (!parenthesisCloseToken) {
        this->addErrorMessageExpected("')' after function call arguments");
        return nullptr;
    }
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(identifierToken->getSourceCodeLocationSpan().start, parenthesisCloseToken->getSourceCodeLocationSpan().end);
    auto identifierNode = std::make_unique<IdentifierNode>(std::move(identifierToken), identifierToken->getSourceCodeLocationSpan());
    auto functionCallExpression = std::make_unique<FunctionCallExpressionNode>(std::move(identifierNode), std::move(arguments), sourceCodeLocationSpan);
    return functionCallExpression;
}

std::unique_ptr<FunctionCallStatementNode> Parser::parseFunctionCallStatement() {
    auto functionCallExpressionNode = this->parseFunctionCallExpression();
    if (!functionCallExpressionNode) {
        this->addErrorMessageParseFailure("function call expression");
        return nullptr;
    }
    auto semicolonToken = this->expectAndAdvance(TokenKind::Semicolon);
    if (!semicolonToken) {
        this->addErrorMessageExpected("';' after function call expression");
        return nullptr;
    }
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(functionCallExpressionNode->getSourceCodeLocationSpan().start, semicolonToken->getSourceCodeLocationSpan().end);
    auto functionCallStatement = std::make_unique<FunctionCallStatementNode>(std::move(functionCallExpressionNode), sourceCodeLocationSpan);
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
    auto semicolonToken = this->expectAndAdvance(TokenKind::Semicolon);
    if (!semicolonToken) {
        this->addErrorMessageExpected("';' after assignment expression");
        return nullptr;
    }
    auto identifierTokenPtr = identifierToken.get();
    auto identifierNode = std::make_unique<IdentifierNode>(std::move(identifierToken), identifierTokenPtr->getSourceCodeLocationSpan());
    auto assignmentSourceCodeLocationSpan = SourceCodeLocationSpan(identifierTokenPtr->getSourceCodeLocationSpan().start, semicolonToken->getSourceCodeLocationSpan().end);
    auto assignmentExpression = std::make_unique<AssignmentExpressionNode>(std::move(identifierNode), std::move(expressionNode), assignmentSourceCodeLocationSpan);
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(identifierTokenPtr->getSourceCodeLocationSpan().start, semicolonToken->getSourceCodeLocationSpan().end);
    auto assignmentStatement = std::make_unique<AssignmentStatementNode>(std::move(assignmentExpression), sourceCodeLocationSpan);
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
    auto identifierNode = std::make_unique<IdentifierNode>(std::move(identifierToken), identifierToken->getSourceCodeLocationSpan());
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(identifierToken->getSourceCodeLocationSpan().start, expressionNode->getSourceCodeLocationSpan().end);
    auto assignmentExpression = std::make_unique<AssignmentExpressionNode>(std::move(identifierNode), std::move(expressionNode), sourceCodeLocationSpan);
    return assignmentExpression;
}

std::unique_ptr<ExpressionNode> Parser::parsePrimaryExpression() {
    Token token = this->peek();
    switch (token.getTokenKind()) {
        case TokenKind::Identifier: {
            if (this->peek(1) == TokenKind::ParenthesisOpen) {
                auto functionCallExpression = this->parseFunctionCallExpression();
                if (!functionCallExpression) {
                    this->addErrorMessageParseFailure("function call expression");
                    return nullptr;
                }
                return functionCallExpression;
            }
            auto identifierToken = this->expectAndAdvance(TokenKind::Identifier);
            auto identifierTokenPtr = identifierToken.get();
            auto sourceCodeLocationSpan = identifierTokenPtr->getSourceCodeLocationSpan();
            auto identifierNode = std::make_unique<IdentifierNode>(std::move(identifierToken), sourceCodeLocationSpan);
            return identifierNode;
        }
        case TokenKind::ParenthesisOpen: {
            this->expectAndAdvance(TokenKind::ParenthesisOpen);
            auto expression = this->parseExpression();
            if (!this->expectAndAdvance(TokenKind::ParenthesisClose)) {
                this->addErrorMessageExpected("')' after parenthesized expression");
                return nullptr;
            }
            return expression; // sourcecodelocationspan ignores parens. OK
        }
        case TokenKind::LiteralBoolean: {
            auto booleanLiteralToken = this->expectAndAdvance(TokenKind::LiteralBoolean);
            std::unique_ptr<BooleanLiteralNode> booleanLiteralNode = std::make_unique<BooleanLiteralNode>(std::move(booleanLiteralToken), booleanLiteralToken->getSourceCodeLocationSpan());
            return booleanLiteralNode;
        }
        case TokenKind::LiteralInteger: {
            auto integerLiteralToken = this->expectAndAdvance(TokenKind::LiteralInteger);
            std::unique_ptr<NumberLiteralNode> integerLiteralNode = std::make_unique<NumberLiteralNode>(std::move(integerLiteralToken), integerLiteralToken->getSourceCodeLocationSpan());
            return integerLiteralNode;
        }
        case TokenKind::LiteralString: {
            auto stringLiteralToken = this->expectAndAdvance(TokenKind::LiteralString);
            std::unique_ptr<StringLiteralNode> stringLiteralNode = std::make_unique<StringLiteralNode>(std::move(stringLiteralToken), stringLiteralToken->getSourceCodeLocationSpan());
            return stringLiteralNode;
        }
        case TokenKind::LiteralEmpty: {
            auto emptyLiteralToken = this->expectAndAdvance(TokenKind::LiteralEmpty);
            std::unique_ptr<EmptyLiteralNode> emptyLiteralNode = std::make_unique<EmptyLiteralNode>(std::move(emptyLiteralToken), emptyLiteralToken->getSourceCodeLocationSpan());
            return emptyLiteralNode;
        }
        case TokenKind::Not: {
            auto operatorToken = this->expectAndAdvance(TokenKind::Not);
            auto operandNode = this->parseExpression();
            if (!operandNode) {
                this->addErrorMessageParseFailure("operand of '!' operator");
                return nullptr;
            }
            auto sourceCodeLocationSpan = SourceCodeLocationSpan(operatorToken->getSourceCodeLocationSpan().start, operandNode->getSourceCodeLocationSpan().end);
            auto unaryOperatorNode = std::make_unique<UnaryOperatorExpressionNode>(std::move(operandNode), std::move(operatorToken), sourceCodeLocationSpan);
            return unaryOperatorNode;
        }
        case TokenKind::Dash: {
            auto operatorToken = this->expectAndAdvance(TokenKind::Dash);
            auto operandNode = this->parseExpression();
            if (!operandNode) {
                this->addErrorMessageParseFailure("operand of '-' operator");
                return nullptr;
            }
            auto sourceCodeLocationSpan = SourceCodeLocationSpan(operatorToken->getSourceCodeLocationSpan().start, operandNode->getSourceCodeLocationSpan().end);
            auto unaryOperatorNode = std::make_unique<UnaryOperatorExpressionNode>(std::move(operandNode), std::move(operatorToken), sourceCodeLocationSpan);
            return unaryOperatorNode;
        }
        default:
            this->addErrorMessageUnexpected("token '" + std::string(token.getSourceString()) + "'");
            return nullptr;
    }
}

std::unique_ptr<ExpressionNode> Parser::parseExpressionClimbing (std::unique_ptr<ExpressionNode> lhs, int minPrecedence) {
    auto lookahead = this->peek();
    while (IS_TOKENKIND_OPERATOR(lookahead.getTokenKind()) && getPrecedence(lookahead.getTokenKind()) >= minPrecedence) {
        auto operatorToken = this->expectAndAdvance(lookahead.getTokenKind());
        std::unique_ptr<ExpressionNode> rhs = this->parsePrimaryExpression();
        if (!rhs) {
            this->addErrorMessageParseFailure("right-hand side expression");
            return nullptr;
        }
        lookahead = this->peek();
        while (
            (IS_TOKENKIND_BINARY_OPERATOR(lookahead.getTokenKind()) && getPrecedence(lookahead.getTokenKind()) > getPrecedence(operatorToken->getTokenKind()))
            ||
            (IS_TOKENKIND_BINARY_OPERATOR(lookahead.getTokenKind()) && getPrecedence(lookahead.getTokenKind()) == getPrecedence(operatorToken->getTokenKind()) && getAssociativity(lookahead.getTokenKind()) == RIGHT_ASSOCIATIVE)
        ) {
            auto nextPrecedenceAddition = getPrecedence(lookahead.getTokenKind()) > getPrecedence(operatorToken->getTokenKind()) ? 1 : 0;
            rhs = this->parseExpressionClimbing(std::move(rhs), getPrecedence(operatorToken->getTokenKind()) + nextPrecedenceAddition);
            if (!rhs) {
                this->addErrorMessageParseFailure("right-hand side expression");
                return nullptr;
            }
            lookahead = this->peek(1);
        }
        if (!IS_TOKENKIND_BINARY_OPERATOR(operatorToken->getTokenKind())) {
            this->addErrorMessageExpected("binary operator");
            return nullptr;
        }
        auto sourceCodeLocationSpan = SourceCodeLocationSpan(lhs->getSourceCodeLocationSpan().start, rhs->getSourceCodeLocationSpan().end);
        lhs = std::make_unique<BinaryOperatorExpressionNode>(std::move(lhs), std::move(rhs), std::move(operatorToken), sourceCodeLocationSpan);
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
    auto sourceCodeLocationSpan = SourceCodeLocationSpan(ifToken->getSourceCodeLocationSpan().start, elseBranchNode->getSourceCodeLocationSpan().end);
    auto ifExpression = std::make_unique<IfExpressionNode>(std::move(conditionNode), std::move(thenBranchNode), std::move(elseBranchNode), sourceCodeLocationSpan);
    return ifExpression;
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() {
    auto token = this->peek();
    if (this->peek() == TokenKind::Identifier && this->peek(1).getTokenKind() == TokenKind::Equal) {
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

std::unique_ptr<TypeExpressionNode> Parser::parseTypeExpression () {
    auto token = this->peek();
    if (token == TokenKind::TypePrimitiveBoolean || token == TokenKind::TypePrimitiveEmpty || token == TokenKind::TypePrimitiveFloat || token == TokenKind::TypePrimitiveInteger || token == TokenKind::TypePrimitiveString) {
        this->expectAndAdvance(token.getTokenKind());
        auto sourceCodeLocationSpan = token.getSourceCodeLocationSpan();
        auto typeExpressionNode = std::make_unique<TypeExpressionNode>(std::make_unique<Token>(token), sourceCodeLocationSpan);
        return typeExpressionNode;
    }
    this->addErrorMessageParseFailure("type expression");
    return nullptr;
}