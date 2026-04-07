#pragma once

#include <deque>

#include "common/sourcecodelocation.hpp"
#include "diagnostics/diagnostics.hpp"
#include "lexer/lexer.hpp"
#include "lexer/token.hpp"
#include "parser/node.hpp"

class Parser {
public:
    Parser(Source* source, Lexer* lexer, Diagnostics& diagnostics) : m_source(source), m_lexer(lexer), m_diagnostics(diagnostics) {};
    std::unique_ptr<Node> parse();
private:
    Source* m_source;
    Lexer* m_lexer;
    Diagnostics& m_diagnostics;
    size_t m_index = 0;
    std::deque<std::unique_ptr<Token>> m_tokenBuffer;
    int m_insideLoop = 0;
    int m_insideFunction = 0;
    int m_insideBlock = 0;
    std::unique_ptr<Token> expectAndAdvance(const TokenKind& expectedTokenName);
    Token peek(size_t offset = 0);
    std::unique_ptr<Token> consumeCurrentToken();
    std::unique_ptr<Token> matchAndAdvance(const TokenKind& expectedTokenName);
    SourceCodeLocationSpan getCurrentSourceCodeLocationSpan();
    bool isPastTokensEnd() const;
    void enterLoop();
    void exitLoop();
    void enterFunction();
    void exitFunction();
    void enterBlock();
    void exitBlock();
    void addErrorMessageParseFailure(const std::string& failedToParse);
    void addErrorMessageExpected(const std::string& expected);
    void addErrorMessageUnexpected(const std::string& unexpected);
    std::unique_ptr<ProgramNode> parseProgram();
    std::unique_ptr<VariableDeclarationNode> parseVariableDeclaration();
    std::unique_ptr<FunctionDeclarationNode> parseFunctionDeclaration();
    std::unique_ptr<IfStatementNode> parseIfStatement();
    std::unique_ptr<WhileStatementNode> parseWhileStatement();
    std::unique_ptr<LoopStatementNode> parseLoopStatement();
    std::unique_ptr<ReturnStatementNode> parseReturnStatement();
    std::unique_ptr<BlockStatementNode> parseBlockStatement();
    std::unique_ptr<ExecutionListNode> parseExecutionList();
    std::unique_ptr<BreakStatementNode> parseBreakStatement();
    std::unique_ptr<ContinueStatementNode> parseContinueStatement();
    std::unique_ptr<FunctionCallStatementNode> parseFunctionCallStatement();
    std::unique_ptr<AssignmentStatementNode> parseAssignmentStatement();
    std::unique_ptr<ExpressionNode> parseExpression();
    std::unique_ptr<ExpressionNode> parsePrimaryExpression();
    std::unique_ptr<TypeExpressionNode> parseTypeExpression();
    std::unique_ptr<IdentifierWithPossibleAnnotationNode> parseIdentifierWithPossibleAnnotation();
    std::unique_ptr<AssignmentExpressionNode> parseAssignmentExpression();
    std::unique_ptr<FunctionCallExpressionNode> parseFunctionCallExpression();
    std::unique_ptr<IfExpressionNode> parseIfExpression();
    std::unique_ptr<ExpressionNode> parseExpressionClimbing(std::unique_ptr<ExpressionNode> lhs, int minPrecedence);
};